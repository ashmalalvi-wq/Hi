#include <Arduino.h>
#include <XboxSeriesXControllerESP32_asukiaaa.hpp>
#include <ESP32Servo.h>

const int servoPWM1 = 16;
const int servoPWM2 = 17;

const int MAX_ACCEL = 300;
const int MIN_ACCEL = 50;

XboxSeriesXControllerESP32_asukiaaa::Core xboxController;
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

    Serial0.print("Current Position: ");
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

    Serial0.print("Current Position: ");
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

void setup() {
    Serial0.begin(115200);
    ESP32PWM::allocateTimer(0);
    goBildaServoHor.setPeriodHertz(50);
    goBildaServoVer.setPeriodHertz(50);
    goBildaServoHor.attach(servoPWM1, 500, 2500);
    goBildaServoVer.attach(servoPWM2, 500, 2500);
    goBildaServoHor.writeMicroseconds(1500);
    goBildaServoVer.writeMicroseconds(1500);
    currentPositionH = 1500; // keep this in sync with the initial write
    currentPositionV = 1500; // keep this in sync with the initial write
    xboxController.begin();
}

void loop() {
    xboxController.onLoop();

    if (xboxController.isConnected()) {
        int joyStickRHori = xboxController.xboxNotif.joyRHori;
        int joyStickRVert = xboxController.xboxNotif.joyRVert;
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