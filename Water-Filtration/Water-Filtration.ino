//turbidity values

unsigned long currentMillis=0;  //Stores the the current time

unsigned long previousMillis = 0;  //Stores the last time the serial monitor was updated

const unsigned long interval = 1000;

int seconds = 0;  //Counter for seconds

 

 

//llsensor

#define LLpin 52   //the liquid level sensor must be attached to pin 52 on your Arduino MEGA for this to work.

bool Level=LOW;

 

//cpump

#include <AFMotor.h>

AF_DCMotor cfpump(2);

 

//servo

#include <Servo.h>

Servo Servo1;

int servo_pos = 0;

int movement_count = 0; // Tracks how many back-and-forth movements are completed

bool completed = false;

 

//DC

AF_DCMotor pump(4);

 

//Peristaltic

AF_DCMotor motor(1);

 

void setup() {

  //turbidity

  Serial.begin(9600);   //Opens serial port and sets rate to 9600 bits per sec. This has to match number on serial monitor!!

  Serial.print("Time\tSensor 1\tSensor2\n");

  Serial.print("====\t========\t=======\n");

  //llsensor

  Serial.begin(9600);            //set up Serial library at 9600 bps -- **Ensure Serial Monitor also has baud rate set to 9600**

  pinMode(LLpin, INPUT_PULLUP);  //Using pinMode you can set digital pins to either read data from the sensor (INPUT) or write data to an actuator (OUTPUT)

                                 //INPUT_PULLUP means that the pin is used to read a sensor, and the signal is kept at HIGH by default

  Serial.print("Reading liquid level sensor:\n");

  //servo

  Servo1.attach(9);

  pinMode(31, INPUT_PULLUP);

  pinMode(35, INPUT_PULLUP);

}

 

//turbidity

void tur() {

    //Get the current time in milliseconds

   currentMillis = millis();

 

     //Check if the interval has passed

  if (currentMillis - previousMillis >= interval) {

     //Save the last time you updated the serial monitor

    previousMillis = currentMillis;

   

     //Increment the seconds counter

    seconds++;

 

  int sensor1 = analogRead(A0); //read the input on analog pin 0

  float voltage1 = sensor1 * (5.0 / 1024.0);  //Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V)

  int sensor2 = analogRead(A1);

  float voltage2 = sensor2 * (5.0 / 1024.0);

  Serial.print(seconds);

  Serial.print("\t");

  Serial.print(voltage1);

  Serial.print("\t\t");

  Serial.println(voltage2);  //println means next loop will print on new line

  }

 

 

}

void lls() {

  Level=digitalRead(LLpin);

  if (Level==HIGH) Serial.print("No object near sensor");

  else if (Level==LOW) Serial.print("Object near sensor");

  Serial.print("\n");  

  delay(1000);

}

void cpump(){

  cfpump.setSpeed(100);

  cfpump.run(FORWARD);

}

 

void servo(){

  // If the servo has completed 4 back-and-forth movements, do nothing

  if (completed) {

    return;

  }

 

  // Perform one full back-and-forth movement

  for (servo_pos = 40; servo_pos <= 125; servo_pos += 1) {

    Servo1.write(servo_pos);

    delay(50);

  }

 

  for (servo_pos = 125; servo_pos >= 40; servo_pos -= 1) {

    Servo1.write(servo_pos);

    delay(50);

  }

 

  // Increment the movement counter

  movement_count++;

 

  // Check if 4 movements are completed

  if (movement_count >= 4) {

    completed = true; // Mark as completed

  }

}

 

void DC() {

  Serial.begin(9600);

  pump.run(FORWARD);             // Change between FORWARD or BACKWARD (all upper case,  Case sensitive) depending on what direction the liquid flows in the pump.

  pump.setSpeed(120);             // Set the speed of pump. Uses an 8 bit value, 255 = 100% speed.

  Serial.println("Pumping!");

}

 

void Persitaltic() {

  Serial.begin(9600);           // set up Serial library at 9600 bps -- **Ensure Serial Monitor also has baud rate set to 9600**

  Serial.print("Starting motor.");

  motor.setSpeed(255);

  motor.run(FORWARD);

}

 

 

void loop() {  

    // Check the start button (pin 36)

    bool startState = digitalRead(35); // Read the button state

    static bool started = false;      // Tracks if the system has started

 

    // If the button is pressed and the system hasn't started yet

    if (startState == LOW && !started) {

        started = true; // Mark the system as started

        Serial.println("System started!");

    }

 

    // If the system hasn't started, do nothing

    if (!started) {

        return;

    }

 

    // Stop button logic

    static bool stopped = false;     // Tracks if the system is stopped

    static bool lastStopState = HIGH; // Tracks the last state of the stop button

    bool stopState = digitalRead(31); // Read the stop button state

 

    // If stop button is pressed

    if (stopState == LOW && lastStopState == HIGH) {

        stopped = true; // Stop the system

        Serial.println("Stop button pressed! System halted.");

    }

    lastStopState = stopState; // Update last stop button state

 

    // If the system is stopped, turn off all motors and servo permanently

    if (stopped) {

        cfpump.setSpeed(0);

        cfpump.run(RELEASE);

 

        pump.setSpeed(0);

        pump.run(RELEASE);

 

        motor.setSpeed(0);

        motor.run(RELEASE);

 

        Servo1.write(40); // Move servo to a neutral position

        return; // Skip the rest of the loop

    }

 

    // Turbidity and liquid level sensor checks

    tur(); // Handle turbidity sensors

    lls(); // Handle liquid level sensor

 

    // Normal operations

    if (Level == HIGH) {

        cpump();

        pump.run(RELEASE);        

        pump.setSpeed(0);

        Servo1.write(40);

    }

    if (Level == LOW) {

        cfpump.setSpeed(0);

        cfpump.run(RELEASE);

        servo();

        DC();

    }

    if (seconds > 45) {

        pump.run(RELEASE);        

        pump.setSpeed(0);

        cfpump.setSpeed(0);

        cfpump.run(RELEASE);

    }

    if (seconds > 105 && seconds < 220) {

      Persitaltic();

    }

}
