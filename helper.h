/*
 * Helper function definitions
 */

#ifndef HELPER
#define HELPER

#include "data.h"
#include "config.h"

void eventLog(byte side, 
              byte type, 
              byte state, 
              unsigned long t)
{
  /*
  Optimized serial print- encoded sensor/actuator identifier with event time
  <byte> side : side identifier
  <byte> type : sensor/actuator identifier
  <byte> state : sensor/actuator state identifier
  <unsigned long> t : event time
  */
  Serial.print(side);
  Serial.print(type);
  Serial.print(state);
  Serial.println(t);
}

unsigned long currentTime(unsigned long tLast = 0, 
                          unsigned long tolerance = CLOCK_TOLERANCE, 
                          bool timeInMicroseconds = TIME_IN_MICROSECONDS)
{
  /*
  Get current time in millis() by default
  <bool> time_in_microseconds : set true to get time in micros()

  Returns:
  <unsigned long> : current time is request format and checked for jumps greater than set tolerance

  CAUTION: millis() can go up to ~49days before overflow where as
           micros() encounters overflow in ~70min
  */
  unsigned long tNow = timeInMicroseconds ? micros() : millis();
  if (tLast != -1) 
  {
    while (tNow - tLast > tolerance || tNow < tLast)
    {
      tNow = timeInMicroseconds ? micros() : millis();
    }
  }
  return tNow;
}

inline bool digitalReadCorrected(byte pin, 
                                 bool sensorLogicLow = false)
{
  /*
  Get logic corrected digital read for IR input pin
  <byte> pin : digital pin ID
  <bool> sensorLogicLow : true if sensor return low for desired readout
  
  Returns : 
  <bool> corrected logic response based on sensor output, not of actual read if sensorLogicLow is true
  */
  bool v = digitalRead(pin);
  return sensorLogicLow ? !v : v;
}

void digitalWriteCorrected(byte pin, 
                           bool state,
                           bool activeLogicLow = false)
{
  /*
  Write logic corrected value to digital pin
  <byte> pin : digital pin to write
  <bool> state : HIGH or LOW, or true or false, or 1 or 0
  */
  digitalWrite(pin, activeLogicLow ? !state : state);
}

void initTTL(TTLState &ttlState,
             byte pin,
             byte mode,
             unsigned long duration = TTL_DURATION,
             unsigned long pulsePeriod = TTL_PULSE_PERIOD,
             unsigned long pulseWidth = TTL_PULSE_WIDTH)
{
  /*
  Initialize default state variable for TTLState
  <struct TTLState> ttlState : struct variable of type TTLState
  <byte> pin : pin to send/receive TTL pulse
  <byte> mode : pin mode - OUTPUT/INPUT_PULLUP/INPUT
  <unsigned long> duration : total duration of TTL signal
	<unsigned long> freq : freq of ttl pulse to be sent for said duration (default in kHz for t in ms or MHz for t in us)
  <unsigned long> pulseWidth : pulseWidth of individual pulse (default in ms or can be in us if TIME_IN_MICROSECONDS) 
  */

  pinMode(pin, mode);
  if (mode == OUTPUT)
  {
    digitalWrite(pin, LOW);
  }
  ttlState.pin = pin;
  ttlState.mode = mode;
  ttlState.state = false;
  ttlState.pulseState = false;
  ttlState.tTTLon = -1;
  ttlState.tPulseon = -1;
  ttlState.duration = duration;
  ttlState.pulseWidth = pulseWidth;
  ttlState.pulsePeriod = pulsePeriod;
};

void updateTTL(TTLState &ttlState, unsigned long tNow)
{
  /*
  Update ttl state

  <struct TTLState> ttlState : struct variable of type TTLState
  <unsigned long> tNow : current time

  NOTE: if ttlState pulseWidth >= pulsePeriod then the TTL pulse remains high through out the set duration
  */
  if (ttlState.state)
  { 
    if ((tNow - ttlState.tTTLon) >= ttlState.duration)
    { 
      digitalWrite(ttlState.pin, LOW);
      ttlState.state = false;
      ttlState.tTTLon = -1;
      ttlState.tPulseon = -1;
    }
    else
    {
      if (ttlState.pulseState)
      { 
        if (((tNow - ttlState.tPulseon) >= ttlState.pulseWidth) && (ttlState.pulseWidth <= ttlState.pulsePeriod))
        {
          digitalWrite(ttlState.pin, LOW);
          ttlState.pulseState = false;
        }
      }
      else 
      {
        if ((tNow - ttlState.tPulseon) >= ttlState.pulsePeriod)
        {
          digitalWrite(ttlState.pin, HIGH);
          ttlState.pulseState = true;
          ttlState.tPulseon = tNow;
        }
      }
    }
  }
}

void sendTTL(TTLState* ttlState, 
             unsigned long tNow, 
             unsigned long pulsePeriod = TTL_PULSE_PERIOD)
{
  /*
  Send a TTL pulse with said freq

  <struct TTLState> ttlState : struct variable of type TTLState
  <unsigned long> tNow : current time
  <unsigned long> freq : freq of ttl pulse
  */
  if (!ttlState->state)
  {
    digitalWrite(ttlState->pin, HIGH);
    ttlState->state = true;
    ttlState->pulseState = true;
    ttlState->tTTLon = tNow;
    ttlState->tPulseon = tNow;
    ttlState->pulsePeriod = pulsePeriod;
  }
}

bool detectTTL(TTLState *ttlState, 
               unsigned long tNow,
               bool completeSquarePulse = false)  
{ 
  /*
  Detect input TTL signal
  <struct TTLState> ttlState : struct variable of type TTLState
  <unsigned long> tNow : current time
  <bool> completeSquarePulse : set to true if the you need detection of completion of square pulse of a specific duration

  Returns:
  <bool> : true if TTL is high in case of completeSquarePulse is set to false, 
           else true only when square pulse of specific duration is detected 
  */
  bool v = digitalRead(ttlState->pin);
  if (!ttlState->state && v)
  {
    ttlState->state = true;
    ttlState->tTTLon = tNow;
    if (!completeSquarePulse) {
      return true;
    }
    return false;
  }
  else if (ttlState->state && !v)
  { 
    unsigned long duration = tNow - ttlState->tTTLon;
    ttlState->state = false;
    ttlState->tTTLon = -1;
    if(duration >= ttlState->pulseWidth)
    {
      return true;
    }
    return false;
  }
  return false;
}

void initRuntime(RuntimeState &runtimeState,
                 byte pin, TTLState* outputTrigger,
                 TTLState* inputTrigger = nullptr,
                 unsigned long duration = RUN_TIME_DURATION,
                 unsigned long delay = DELAY_START)
{
  /*
  Initialize default state variable for runtime
  <struct RuntimeState> runtimeState : runtime struct variable
  <byte> pin : led indicator pin for runtime - HIGH when on, LOW when off
  <unsigned long> duration : set total duration for runtime execution, defaults to RUN_TIME_DURATION
  */
  pinMode(pin, OUTPUT);
  runtimeState.led_pin  = pin;
  runtimeState.runtimeFlag = false;
  runtimeState.duration = duration;
  runtimeState.delay = delay;
  digitalWrite(runtimeState.led_pin, OFF);
  runtimeState.tStart = currentTime(-1);
  runtimeState.tLast = -1;
  runtimeState.inputTrigger = inputTrigger;
  runtimeState.outputTrigger = outputTrigger;
}

void updateRuntime(RuntimeState &runtimeState)
{
  /*
  Poll for current time and check for start or exit conditions for runtime
  <struct RuntimeState> runtimeState : runtime struct variable
  */
  runtimeState.tNow = currentTime(runtimeState.tLast);
  if (runtimeState.inputTrigger == nullptr)
  {
    //exit condition
    if (runtimeState.runtimeFlag && (runtimeState.tNow - runtimeState.tRuntimeStart >= runtimeState.duration))
    {
      digitalWrite(runtimeState.led_pin, OFF);
      digitalWriteCorrected(SOLENOID_A_PIN, OFF, SOLENOID_ACTIVE_LOW);
      digitalWriteCorrected(SOLENOID_B_PIN, OFF, SOLENOID_ACTIVE_LOW);
      runtimeState.runtimeFlag = false;
      // log
      Serial.print('E');
      Serial.println(runtimeState.tNow);

      while (true);
    }
    //start condition
    if (!runtimeState.runtimeFlag && runtimeState.tNow - runtimeState.tStart >= DELAY_START)
    {
      runtimeState.runtimeFlag = true;
      digitalWrite(runtimeState.led_pin, ON);
      runtimeState.tRuntimeStart = runtimeState.tNow;
      // log
      Serial.print('S');
      Serial.println(runtimeState.tRuntimeStart);
    }
  }
  else
  { 
    bool inputTrigger = detectTTL(runtimeState.inputTrigger, runtimeState.tNow);
    if (runtimeState.runtimeFlag && (runtimeState.tNow - runtimeState.tRuntimeStart >= runtimeState.duration))
    {
      digitalWrite(runtimeState.led_pin, OFF);
      digitalWriteCorrected(SOLENOID_A_PIN, OFF, SOLENOID_ACTIVE_LOW);
      digitalWriteCorrected(SOLENOID_B_PIN, OFF, SOLENOID_ACTIVE_LOW);
      runtimeState.runtimeFlag = false;
      sendTTL(runtimeState.outputTrigger, runtimeState.tNow);
      // log
      Serial.print('E');
      Serial.println(runtimeState.tNow);
    }
    if (inputTrigger && !runtimeState.runtimeFlag)
    {
      runtimeState.runtimeFlag = true;
      digitalWrite(runtimeState.led_pin, ON);
      runtimeState.tRuntimeStart = runtimeState.tNow;
      // log
      Serial.print('S');
      Serial.println(runtimeState.tRuntimeStart);
    }
  }
  runtimeState.tLast = runtimeState.tNow;
}

void initBlinkLED(BlinkLEDState &ledState,
                  byte pin,
                  byte side,
                  unsigned long blinkInterval = LED_BLINK_INTERVAL)
{
  /*
  Initialize default parameters for blinking LED
  <struct BlinkLEDState> ledState : struct for led state parameters
  <byte> side : led location/side identifier
  <byte> pin : led pin
  <unsigned long> blinkInterval : blink duration
  */
  pinMode(pin, OUTPUT);
  ledState.pin = pin;
  ledState.side = side;
  ledState.tLEDon = 0;
  ledState.tLEDoff = 0;
  ledState.ledBlinkState = false; 
  ledState.blinkInterval = blinkInterval;
}

void updateBlinkLED(BlinkLEDState &ledState,
                    unsigned long tNow)
{
  /*
  update led blink between on and off
  <struct BlinkLEDState> ledState : struct for led state parameters
  <unsigned long> tNow : current time
  */
  if (ledState.ledBlinkState && (tNow - ledState.tLEDon > ledState.blinkInterval))
  {
    digitalWrite(ledState.pin, OFF);
    ledState.tLEDoff = tNow;
    ledState.ledBlinkState = false;
  }
  if (!ledState.ledBlinkState && (tNow - ledState.tLEDoff > ledState.blinkInterval))
  {
    digitalWrite(ledState.pin, ON);
    ledState.tLEDon = tNow;
    ledState.ledBlinkState = true;
  }

}

void initIR(IRState &irDetector,
            byte pin,
            byte side,
            byte proxyLEDPin,
            TTLState* outputTrigger,
            unsigned long ttlPulsePeriod = TTL_PULSE_PERIOD)
{
  /*
  Init function to initialize irDetector with default parameters
  <IRState> irDetector : struct storing irDetector state parameters
  <byte> pin : input pin ID connected to ir sensor
  <byte> side : side identifier
  */
  pinMode(pin, INPUT_PULLUP);
  pinMode(proxyLEDPin, OUTPUT);
  digitalWrite(proxyLEDPin, LOW);
  irDetector.pin = pin;
  irDetector.side = side;
  irDetector.proxyLEDPin = proxyLEDPin;
  irDetector.currentRead = digitalReadCorrected(pin, IR_ACTIVE_LOW);
  irDetector.lastRead = false;
  irDetector.currentPersistant = false;
  irDetector.lastPersistant = false;
  irDetector.inBreak = false;
  irDetector.breakEvent = false;
  irDetector.breakEventMutable = false;
  irDetector.connectEvent = true;
  irDetector.outputTrigger = outputTrigger;
  irDetector.ttlPulsePeriod = ttlPulsePeriod;
}

void detectIR(IRState &irDetector,
              unsigned long tNow)
{
  /*
  Function to detect irDetector state changes and update state parameters accordingly
    IR state change event is recorded following persistance in signal
    Additionally since alternating high-low sig was detected when IR emitter and detector
    were placed inside a circular housing within the behavior setup, || login between current and last read is implemented

  <IRState> irDetector : struct storing irDetector state parameters
  <unsigned long> tNow : current time of execution
  */
  bool v = digitalReadCorrected(irDetector.pin, IR_ACTIVE_LOW);
  if ((irDetector.currentRead || irDetector.lastRead) && v && !irDetector.inBreak)
  {
    irDetector.tStart = tNow;
    irDetector.inBreak = true;
  }
  else if (!(irDetector.currentRead || irDetector.lastRead) && !v && irDetector.inBreak)
  {
    irDetector.tOff = tNow;
    irDetector.inBreak = false;
  }
  if (tNow - irDetector.tOff >= MIN_IR_BREAK && !irDetector.inBreak && irDetector.breakEvent)
  {
    irDetector.inBreak = false;
    irDetector.breakEvent = false;
    irDetector.breakEventMutable = false;
    irDetector.connectEvent = true;
    // log
    eventLog(irDetector.side, IR, OFF, tNow);
    digitalWrite(irDetector.proxyLEDPin, LOW);
  }
  if (tNow - irDetector.tStart >= MIN_IR_BREAK && irDetector.inBreak && irDetector.connectEvent)
  {
    irDetector.breakEvent = true;
    irDetector.breakEventMutable = true;
    irDetector.connectEvent = false;
    // log
    eventLog(irDetector.side, IR, ON, tNow);
    digitalWrite(irDetector.proxyLEDPin, HIGH);
    sendTTL(irDetector.outputTrigger, tNow, irDetector.ttlPulsePeriod);
  }
  if (irDetector.breakEventMutable)
  {
    irDetector.lastPersistant = irDetector.currentPersistant;
    irDetector.currentPersistant = true;
    irDetector.breakEventMutable = false;
  }
  irDetector.lastRead = irDetector.currentRead;
  irDetector.currentRead = v;
}

void initTouch(TouchState &touchSensor,
               byte pin,
               byte side,
               TTLState* outputTrigger,
               unsigned long ttlPulsePeriod = TTL_PULSE_PERIOD)
{
  /*
  Init function to initialize touchSensor with default parameters
  <TouchState> touchSensor : struct storing touchSensor state parameters
  <byte> pin : input pin ID connected to touch sensor
  <byte> side : side identifier
  */
  pinMode(pin, INPUT_PULLUP);
  touchSensor.pin = pin;
  touchSensor.side = side;
  touchSensor.current = digitalReadCorrected(touchSensor.pin, TOUCH_ACTIVE_LOW);
  touchSensor.last = false;
  touchSensor.inTouch = false;
  touchSensor.touchEvent = false;
  touchSensor.clearEvent = true;
  touchSensor.outputTrigger = outputTrigger;
  touchSensor.ttlPulsePeriod = ttlPulsePeriod;
}

void detectTouch(TouchState &touchSensor,
                 unsigned long tNow)
{
  /*
  Function to detect touchSensor state changes and update state parameters accordingly
  <TouchState> touchSensor : struct storing irDetector state parameters
  <unsigned long> tNow : current time of execution
  <unsigned long> tRuntimeStart : time of runtime start
  */
  bool v = digitalReadCorrected(touchSensor.pin, TOUCH_ACTIVE_LOW);

  if (v && !touchSensor.last)
  {
    // add state change as and when needed for duration dependent reward release
    touchSensor.tStart = tNow;
    touchSensor.inTouch = true;
    touchSensor.touchEvent = true;
    touchSensor.clearEvent = false;
    // log
    eventLog(touchSensor.side, TOUCH, ON, tNow);
    sendTTL(touchSensor.outputTrigger, tNow, touchSensor.ttlPulsePeriod);
  }
  else if (!v && touchSensor.last)
  {
    touchSensor.inTouch = false;
    touchSensor.clearEvent = true;
    touchSensor.touchEvent = false;
    // log
    eventLog(touchSensor.side, TOUCH, OFF, tNow);
  }
  touchSensor.last = v;
}

void initSolenoid(SolenoidState &solenoidValve,
                  byte pin,
                  byte side,
                  TTLState* outputTrigger,
                  unsigned long ttlPulsePeriod = TTL_PULSE_PERIOD)
{
  /*
  Init function to initialize solenoidState with default parameters
  <SolenoidState> solenoidValve : struct storing solenoid valve state parameters
  <byte> pin : input pin ID connected to touch sensor
  <byte> side : side identifier
  */
  pinMode(pin, OUTPUT);
  digitalWriteCorrected(pin, OFF, SOLENOID_ACTIVE_LOW);
  solenoidValve.pin = pin;
  solenoidValve.side = side;
  solenoidValve.open = false;
  solenoidValve.outputTrigger = outputTrigger;
  solenoidValve.ttlPulsePeriod = TTL_PULSE_PERIOD;
}

void activateSolenoid(SolenoidState &solenoidValve,
                      unsigned long tNow,
                      unsigned long duration = SOLENOID_DURATION)
{
  /*
  Function to activate solenoid valve and update state parameters accordingly
  <SolenoidState> solenoidValve : struct storing solenoid valve state parameters
  <unsigned long> duration : duration to keep the solenoid valve open/close depending on type of solenoid
  <unsigned long> tNow : current time of execution
  <unsigned long> tRuntimeStart : time of runtime start
  */
  if (solenoidValve.open)
  {
    return;
  }
  if (!solenoidValve.open)
  {
    solenoidValve.open = true;
    solenoidValve.tOpen = tNow;
    solenoidValve.duration = duration;
    digitalWriteCorrected(solenoidValve.pin, ON, SOLENOID_ACTIVE_LOW);

    // log
    eventLog(solenoidValve.side, SOLENOID, ON, tNow);
    sendTTL(solenoidValve.outputTrigger, tNow, solenoidValve.ttlPulsePeriod);
  }
}

void updateSolenoid(SolenoidState &solenoidValve,
                    unsigned long tNow)
{
  /*
  Function to check for duration elapsed since solenoid valve activation and update state parameters accordingly
  <SolenoidState> solenoidValve : struct storing solenoid valve state parameters
  <unsigned long> tNow : current time of execution
  <unsigned long> tRuntimeStart : time of runtime start
  */
  if (solenoidValve.open && tNow - solenoidValve.tOpen >= solenoidValve.duration)
  {
    solenoidValve.open = false;
    solenoidValve.tClose = tNow;
    digitalWriteCorrected(solenoidValve.pin, OFF, SOLENOID_ACTIVE_LOW);
    // log
    eventLog(solenoidValve.side, SOLENOID, OFF, tNow);
  }
}

#endif