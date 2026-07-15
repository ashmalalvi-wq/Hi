#include <Arduino.h>
#include <XboxSeriesXControllerESP32_asukiaaa.hpp>

// Define the control pins for Motor Channel A
const int PWM_A = 4;  // Speed control pin
const int AIN_1 = 5;  // Direction pin 1
const int AIN_2 = 6;  // Direction pin 2

// Maximum PWM value for safe full speed
const int MAX_PWM = 255*6/7.4; // Maximum PWM value for full speed 

// any xbox controller
XboxSeriesXControllerESP32_asukiaaa::Core xboxController;

void controlMotor(int triggerL, int triggerR) {
    // Calculate the speed based on the triggers divided by 1023 to normalize the value, then multiplied by MAX_PWM
    int speed = (triggerR - triggerL)/1023 * MAX_PWM; // Right trigger for forward, left trigger for reverse

    // Constrain the speed to the maximum PWM value
    speed = constrain(speed, -MAX_PWM, MAX_PWM);

    if (speed > 0) {
        // Forward direction
        analogWrite(PWM_A, speed);
        digitalWrite(AIN_1, HIGH);
        digitalWrite(AIN_2, LOW);
    } else if (speed < 0) {
        // Reverse direction
        analogWrite(PWM_A, -speed); // Use the absolute value for PWM
        digitalWrite(AIN_1, LOW);
        digitalWrite(AIN_2, HIGH);
    } else {
        // Stop the motor
        analogWrite(PWM_A, 0);
        digitalWrite(AIN_1, LOW);
        digitalWrite(AIN_2, LOW);
    }
}

void setup() {
    // Initialize all motor control pins as outputs
    pinMode(PWM_A, OUTPUT);
    pinMode(AIN_1, OUTPUT);
    pinMode(AIN_2, OUTPUT);
    xboxController.begin();
}

void loop() {
    xboxController.onLoop();

    if (xboxController.isConnected()) {
            int triggerL = xboxController.xboxNotif.trigLT;
            int triggerR = xboxController.xboxNotif.trigRT;
            controlMotor(triggerL, triggerR);
        }
    else {
        Serial0.println("not connected");
        if (xboxController.getCountFailedConnection() > 2) {
            ESP.restart();
        }
    }

    
    delay(2000); // Stay stopped for 2 seconds
}