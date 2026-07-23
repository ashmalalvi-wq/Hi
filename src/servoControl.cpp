#include <Arduino.h>
#include <XboxSeriesXControllerESP32_asukiaaa.hpp>
#include <ESP32Servo.h>

const int servoPWM1 = 16;

const int MAX_ACCEL = 300;
const int MIN_ACCEL = 50;

XboxSeriesXControllerESP32_asukiaaa::Core xboxController;
Servo goBildaServo;

// Track the servo's commanded position ourselves — don't rely on readMicroseconds()
int currentPosition = 1500;

void controlServoMotor(int rightStick){
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

    int newPosition = currentPosition + camHorizontalAccel;
    newPosition = constrain(newPosition, 500, 2500);

    Serial0.print("Current Position: ");
    Serial0.println(currentPosition);
    Serial0.print("New Position: ");
    Serial0.println(newPosition);

    goBildaServo.writeMicroseconds(newPosition);
    currentPosition = newPosition; // update our own tracked state, not from the servo lib
}

void setup() {
    Serial0.begin(115200);
    ESP32PWM::allocateTimer(0);
    goBildaServo.setPeriodHertz(50);
    goBildaServo.attach(servoPWM1, 500, 2500);
    goBildaServo.writeMicroseconds(1500);
    currentPosition = 1500; // keep this in sync with the initial write
    xboxController.begin();
}

void loop() {
    xboxController.onLoop();

    if (xboxController.isConnected()) {
        int joyStickR = xboxController.xboxNotif.joyRHori;
        controlServoMotor(joyStickR);
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