#include <Arduino.h>
#include <XboxSeriesXControllerESP32_asukiaaa.hpp>
#include <ESP32Servo.h>

// Servo motor control pins
const int servoPWM1 = 17;
const int servoPWM2 = 18;
//Servo motor accel limits
const int MAX_ACCEL = 300;
const int MIN_ACCEL = 50;

// Move Motor Channel A pins
const int PWM_A = 4;
const int AIN_1 = 5;
const int AIN_2 = 6;

// Move Motor Channel B pins
const int PWM_B = 7;
const int BIN_1 = 15;
const int BIN_2 = 16;

const int MAX_PWM = 255;

// Xbox controller object
XboxSeriesXControllerESP32_asukiaaa::Core xboxController;

// Servo motor objects
Servo goBildaServoHor;
Servo goBildaServoVer;

// Track the servo's commanded position ourselves — don't rely on readMicroseconds()
int currentPositionH = 1500;
int currentPositionV = 1500;

void controlServoMotorHor(int rightStick){
    int camHorizontalAccel{};
    if (rightStick > 38000){
        camHorizontalAccel = map(rightStick, 38000, 65535, MIN_ACCEL, MAX_ACCEL);
    }
    else if (rightStick < 30000){
        camHorizontalAccel = map(rightStick, 0, 30000, -MAX_ACCEL, -MIN_ACCEL);
    }
    else{
        camHorizontalAccel = 0;
    }

    int newPosition = currentPositionH + camHorizontalAccel;
    newPosition = constrain(newPosition, 500, 2500);

    Serial0.print("Current Position Horizontal: ");
    Serial0.println(currentPositionH);
    Serial0.print("New Position: ");
    Serial0.println(newPosition);

    goBildaServoHor.writeMicroseconds(newPosition);
    currentPositionH = newPosition; // update our own tracked state, not from the servo lib
}
void controlServoMotorVer(int rightStick){

    int camVerticalAccel{};
    if (rightStick > 38000){
        camVerticalAccel = map(rightStick, 38000, 65535, MIN_ACCEL, MAX_ACCEL);
    }
    else if (rightStick < 30000){
        camVerticalAccel = map(rightStick, 0, 30000, -MAX_ACCEL, -MIN_ACCEL);
    }
    else{
        camVerticalAccel = 0;
    }

    int newPosition = currentPositionV + camVerticalAccel;
    newPosition = constrain(newPosition, 500, 2500);

    Serial0.print("Current Position Vertical: ");
    Serial0.println(currentPositionV);
    Serial0.print("New Position: ");
    Serial0.println(newPosition);

    goBildaServoVer.writeMicroseconds(newPosition);
    currentPositionV = newPosition; // update our own tracked state, not from the servo lib
}

void controlServoMotor(int rightStickHor, int rightStickVert){
    controlServoMotorHor(rightStickHor);
    controlServoMotorVer(rightStickVert);
}

// Writes a signed speed (-MAX_PWM to MAX_PWM) to one motor.
// speed > 0 = forward, speed < 0 = reverse, speed == 0 = active brake
void driveMotor(int pwmPin, int in1Pin, int in2Pin, int speed){
    speed = constrain(speed, -MAX_PWM, MAX_PWM);
    int absSpeed = abs(speed);

    if (speed > 0) {
        analogWrite(pwmPin, absSpeed);
        digitalWrite(in1Pin, HIGH);
        digitalWrite(in2Pin, LOW);
    }
    else if (speed < 0) {
        analogWrite(pwmPin, absSpeed);
        digitalWrite(in1Pin, LOW);
        digitalWrite(in2Pin, HIGH);
    }
    else {
        analogWrite(pwmPin, 0);
        digitalWrite(in1Pin, HIGH);
        digitalWrite(in2Pin, HIGH); // active brake
    }
}

void controlMotor(int triggerL, int triggerR, int leftStick){
    // --- Drive speed from triggers ---
    int netTrigger = triggerR - triggerL;
    int driveSpeed = map(netTrigger, -1023, 1023, -MAX_PWM, MAX_PWM);
    driveSpeed = constrain(driveSpeed, -MAX_PWM, MAX_PWM);

    // --- Turn speed from left stick ---
    int turnSpeed = 0;
    if (leftStick > 38000){
        turnSpeed = map(leftStick, 38000, 65535, 0, MAX_PWM);   // turn right
    }
    else if (leftStick < 32000){
        turnSpeed = map(leftStick, 0, 32000, -MAX_PWM, 0);      // turn left
    }
    turnSpeed = constrain(turnSpeed, -MAX_PWM, MAX_PWM);

    // --- Mix drive + turn per side ---
    int leftMotorSpeed  = driveSpeed + turnSpeed;
    int rightMotorSpeed = driveSpeed - turnSpeed;

    // Re-constrain after mixing, since the sum can exceed MAX_PWM
    leftMotorSpeed  = constrain(leftMotorSpeed,  -MAX_PWM, MAX_PWM);
    rightMotorSpeed = constrain(rightMotorSpeed, -MAX_PWM, MAX_PWM);

    driveMotor(PWM_A, AIN_1, AIN_2, leftMotorSpeed);
    driveMotor(PWM_B, BIN_1, BIN_2, rightMotorSpeed);

    Serial0.print("Drive:"); Serial0.print(driveSpeed);
    Serial0.print(" Turn:"); Serial0.print(turnSpeed);
    Serial0.print(" L:"); Serial0.print(leftMotorSpeed);
    Serial0.print(" R:"); Serial0.println(rightMotorSpeed);
}

void setup() {
    //Serial0.begin(115200);

    // Servo motor setup
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    goBildaServoHor.setPeriodHertz(50);
    goBildaServoVer.setPeriodHertz(50);
    goBildaServoHor.attach(servoPWM1, 500, 2500);
    goBildaServoVer.attach(servoPWM2, 500, 2500);
    goBildaServoHor.writeMicroseconds(1500);
    goBildaServoVer.writeMicroseconds(1500);
    currentPositionH = 1500; // keep this in sync with the initial write
    currentPositionV = 1500; // keep this in sync with the initial write

    // Move motor pin setup
    pinMode(PWM_A, OUTPUT);
    pinMode(AIN_1, OUTPUT);
    pinMode(AIN_2, OUTPUT);

    pinMode(PWM_B, OUTPUT);
    pinMode(BIN_1, OUTPUT);
    pinMode(BIN_2, OUTPUT);


    xboxController.begin();
}

void loop() {
    xboxController.onLoop();

    if (xboxController.isConnected()) {
        int joyStickRHori = xboxController.xboxNotif.joyRHori;
        int joyStickRVert = xboxController.xboxNotif.joyRVert;
        int triggerL = xboxController.xboxNotif.trigLT;
        int triggerR = xboxController.xboxNotif.trigRT;
        int joyStickL = xboxController.xboxNotif.joyLHori;

        controlMotor(triggerL, triggerR, joyStickL);
        controlServoMotor(joyStickRHori, joyStickRVert);
        delay(100);
    }
    else {
        Serial0.println("not connected");
        if (xboxController.getCountFailedConnection() > 2) {
            ESP.restart();
        }
        delay(500);
    }
}