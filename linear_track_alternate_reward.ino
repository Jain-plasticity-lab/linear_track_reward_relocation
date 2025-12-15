#include "config.h"
#include "data.h"
#include "helper.h"

int lastIR;

RuntimeState runtime;
BlinkLEDState ledA;
IRState irDetectorA, irDetectorB;
TouchState touchSensorA, touchSensorB;
SolenoidState solenoidValveA, solenoidValveB;

void setup()
{
  lastIR = -1;
  Serial.begin(BAUD_RATE);
  initRuntime(runtime, LED_RUNTIME);
  delay(1001); // to allow serial conenction to be established
  initBlinkLED(ledA, LED_BLINK_PIN, SIDE_A);
  initIR(irDetectorA, IR_A_PIN, SIDE_A);
  initIR(irDetectorB, IR_B_PIN, SIDE_B);
  initTouch(touchSensorA, TOUCH_A_PIN, SIDE_A);
  initTouch(touchSensorB, TOUCH_B_PIN, SIDE_B);
  initSolenoid(solenoidValveA, SOLENOID_A_PIN, SIDE_A);
  initSolenoid(solenoidValveB, SOLENOID_B_PIN, SIDE_B);
  // log
  Serial.print("Linear Track Behaviour in mode: ");
  OPERATION_MODE ? Serial.println("Mode_B") : Serial.println("Mode_A");
}

void loop()
{
  updateRuntime(runtime);
  if (runtime.runTimeFlag)
  { 
    detectIR(irDetectorA, runtime.tNow);
    detectIR(irDetectorB, runtime.tNow);
    detectTouch(touchSensorA, runtime.tNow);
    detectTouch(touchSensorB, runtime.tNow);

    updateBlinkLED(ledA, runtime.tNow);
    updateSolenoid(solenoidValveA, runtime.tNow);
    updateSolenoid(solenoidValveB, runtime.tNow);

    

    switch (OPERATION_MODE)
    {
      case MODE_A:
        if (irDetectorA.currentPersistant && (lastIR == SIDE_B))
        {
          activateSolenoid(solenoidValveA, runtime.tNow);
          activateSolenoid(solenoidValveB, runtime.tNow);//ensure the reservoir inlet valve is closed
        }
        break;
      case MODE_B:
        if (irDetectorB.currentPersistant && (lastIR == SIDE_A))
        {
          activateSolenoid(solenoidValveB, runtime.tNow);
          activateSolenoid(solenoidValveA, runtime.tNow);//ensure the reservoir inlet valve is closed
        }
      default:
        Serial.println("Operation Mode configuration incorrect/incomplete");
        break;
    }
    if (irDetectorA.currentPersistant) 
    {
      irDetectorA.lastPersistant = irDetectorA.currentPersistant;
      irDetectorA.currentPersistant = false;
      irDetectorB.lastPersistant = false;
      lastIR = SIDE_A;
    }
    else if (irDetectorB.currentPersistant)
    {
      irDetectorB.lastPersistant = irDetectorB.currentPersistant;
      irDetectorB.currentPersistant = false;
      irDetectorA.lastPersistant = false;
      lastIR = SIDE_B;
    }
  }
  runtime.tLast = runtime.tNow;
}