/*  Define all your connections/pins into the data acquisition board
 *   and other relevant globals(Arduino Uno?), include the folder in your arduino main
 *   5-IR,6-touch,7-solenoid  -side A (0)
 *   8-IR,9-touch,10-solenoid - side B (1)
 */

#ifndef CONFIG
#define CONFIG

#include "data.h"

/*
 * Timing mode
 * defaults to millis which can run upto ~49days before encountering overflow and restarts from 0
 * beware while using micros as overflow occurs rather quickly at around 70min and restarts from 0
 */
const bool TIME_IN_MICROSECONDS = false;

/*Operation Mode*/
// change manually between trial MODE_A for reward at A and MODE_B for reward at B runs to switch reward location as required
const enum Mode OPERATION_MODE = MODE_A;

/*Pins*/
const byte INPUT_TRIGGER = A0; //Connected to output_1 of RWD 810 fiberphotometery, received upon acquisition start
const byte OUTPUT_TRIGGER = A1; //Connected to input_1 of RWD 810 fiberphotometery, sent upon runtime completion to stop acquisition

const byte IR_A_INDICATOR = A2;
const byte IR_B_INDICATOR = A3;

const byte OUTPUT_IR = 2; //Connected to input_2 of RWD 810 fiberphotometery
const byte OUTPUT_TOUCH = 3;  //Connected to input_3 of RWD 810 fiberphotometery
const byte OUTPUT_SOLENOID = 4; //Connected to input_4 of RWD 810 fiberphotometery

const byte IR_A_PIN = 5;
const byte TOUCH_A_PIN = 6;
const byte SOLENOID_A_PIN = 7;

const byte IR_B_PIN = 8;
const byte TOUCH_B_PIN = 9;
const byte SOLENOID_B_PIN = 10;

const byte LED_BLINK_PIN = 12;
const byte LED_RUNTIME = 13;


// Serial transfer baud rate;
const unsigned long BAUD_RATE = 9600UL;

/*Identifiers for serial data transfer*/
const byte SIDE_A = 0;
const byte SIDE_B = 1;
const byte OFF = 0;
const byte ON = 1;
const byte IR = 0;
const byte TOUCH = 1;
const byte SOLENOID = 2;

/*Sensor state indicator logic*/
const bool IR_ACTIVE_LOW = false;
const bool TOUCH_ACTIVE_LOW = false;
const bool SOLENOID_ACTIVE_LOW = true;

/*Time parameters - type dependent on tNow parameter in*/
const unsigned long CLOCK_TOLERANCE = 20UL * (1 + (TIME_IN_MICROSECONDS * (1000 - 1)));         //tolerance range if timing function jumps? would subsequent calls be resolved?

const unsigned long TTL_DURATION = 50UL * (1 + (TIME_IN_MICROSECONDS * (1000 - 1)));
const unsigned long TTL_PULSE_WIDTH = 20UL * (1 + (TIME_IN_MICROSECONDS * (1000 - 1)));
const unsigned long TTL_PULSE_PERIOD = 50UL * (1 + (TIME_IN_MICROSECONDS * (1000 - 1)));
// const unsigned long TTL_TOLERANCE = 5UL * (1 + (TIME_IN_MICROSECONDS * (1000 - 1)));

const unsigned long DELAY_START = 4UL * 1000UL * (1 + (TIME_IN_MICROSECONDS * (1000 - 1)));      // time to start void loop()
const unsigned long RUN_TIME_DURATION = 20UL * 60UL * 1000UL * (1 + (TIME_IN_MICROSECONDS * (1000 - 1))); // time since above delay completion

const unsigned long MIN_IR_BREAK = 5UL * (1 + (TIME_IN_MICROSECONDS * (1000 - 1)));             // duration for signal persistance to avoid transient spike
const unsigned long SOLENOID_DURATION = 40UL * (1 + (TIME_IN_MICROSECONDS * (1000 - 1)));       // duration of solenoid valve release
const unsigned long LED_BLINK_INTERVAL = 500UL * (1 + (TIME_IN_MICROSECONDS * (1000 - 1)));     // led blink on interval

#endif