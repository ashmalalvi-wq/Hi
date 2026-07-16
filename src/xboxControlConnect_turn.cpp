#include <Arduino.h>
#include <XboxSeriesXControllerESP32_asukiaaa.hpp>

// Define the control pins for Motor Channel A
const int PWM_A = 4;  // Speed control pin
const int AIN_1 = 5;  // Direction pin 1
const int AIN_2 = 6;  // Direction pin 2

// Maximum PWM value for safe full speed
const int MAX_PWM = 255*7.4/7.4; // Maximum PWM value for full speed

// any xbox controller
XboxSeriesXControllerESP32_asukiaaa::Core xboxController;

void controlMotor(int triggerL, int triggerR) {
    // Calculate the speed based on the triggers divided by 1023 (max trigger), then multiplied by MAX_PWM
    int netTrigger = triggerR - triggerL;
    // Map the net trigger value from the range of -1023 to 1023 to the range of -MAX_PWM to MAX_PWM
    int speed = map(netTrigger, -1023, 1023, (-MAX_PWM), MAX_PWM);

    // Constrain the speed to the maximum PWM value
    speed = constrain(speed, -MAX_PWM, MAX_PWM);
    int absSpeed = abs(speed);

    if (speed > 0) {
        // Forward
        analogWrite(PWM_A, absSpeed);
        digitalWrite(AIN_1, HIGH);
        digitalWrite(AIN_2, LOW);
    } 
    else if (speed < 0) {
        // Reverse
        analogWrite(PWM_A, absSpeed);
        digitalWrite(AIN_1, LOW);
        digitalWrite(AIN_2, HIGH);
    } 
    else {
        // Active Short Brake (Stronger, cleaner stop for robotics than Coast)
        analogWrite(PWM_A, 0);
        digitalWrite(AIN_1, HIGH);
        digitalWrite(AIN_2, HIGH);
    }

    // // Correct Debug Printing
    // Serial0.print("SP:");   Serial0.println(speed);
    // Serial0.print("A1:");   Serial0.println(digitalRead(AIN_1));
    // Serial0.print("A2:");   Serial0.println(digitalRead(AIN_2));

}

void controlTurn(int leftStick){
    // Calculate turn speed based on the left stick horizontal position with 0 as left and 65535 as right (max stick value)
    int turnSpeed;
    if (leftStick > 38000){
        turnSpeed = map(leftStick, 38000, 65535, 0, MAX_PWM);
    }
    else if (leftStick < 32000){
        turnSpeed = map(leftStick, 0, 32000, -MAX_PWM, 0);
    }
    else{
        turnSpeed = 0;
    }

    // Constrain the speed to the maximum PWM value
    turnSpeed = constrain(turnSpeed, -MAX_PWM, MAX_PWM);
    int absSpeed = abs(turnSpeed);

    if (turnSpeed > 0) {
        // Turn Right
        analogWrite(PWM_A, absSpeed);
        digitalWrite(AIN_1, HIGH);
        digitalWrite(AIN_2, LOW);
    } 
    else if (turnSpeed < 0) {
        // Turn Left
        analogWrite(PWM_A, absSpeed);
        digitalWrite(AIN_1, LOW);
        digitalWrite(AIN_2, HIGH);
    } 
}

void setup() {
    Serial0.begin(115200);
    // Initialize all motor control pins as outputs
    pinMode(PWM_A, OUTPUT);
    pinMode(AIN_1, OUTPUT);
    pinMode(AIN_2, OUTPUT);
    xboxController.begin();
}

void loop() {
    xboxController.onLoop();

    if (xboxController.isConnected()) {
            int joyStickL = xboxController.xboxNotif.joyLHori;
            int joyStickR = xboxController.xboxNotif.joyRHori;
            Serial0.print("Joy Stick L: ");
            Serial0.print(joyStickL);

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