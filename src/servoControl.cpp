#include <Arduino.h>
#include <XboxSeriesXControllerESP32_asukiaaa.hpp>
#include <ESP32Servo.h>

// Define the control pins for Motor Channel A
const int servoPWM1 = 16;  // Speed control pin

// any xbox controller
XboxSeriesXControllerESP32_asukiaaa::Core xboxController;

//Servo object
Servo goBildaServo;

void controlServoMotor(int rightStick){
    // Map the right stick horizontal position (0 to 1023) to acceleration values for the servo to move between 500 to 2500 microseconds
   int camHorizontalAccel{};
   if (rightStick > 38000){
        camHorizontalAccel = map(rightStick, 38000, 65535, 0, 500);
    }
    else if (rightStick < 30000){
        camHorizontalAccel = map(rightStick, 0, 30000, -500, 0);
    }
    else{
        camHorizontalAccel  = 0;
    }
    Serial0.print("Right Stick: ");
    Serial0.println(rightStick);
    Serial0.print("Cam Horizontal Accel: ");
    Serial0.println(camHorizontalAccel);
    // Read the current position of the servo in microseconds
    // int currentPosition = goBildaServo.readMicroseconds();
    // while (currentPosition > 500 && currentPosition < 2500) {
    //     // Calculate the new position based on the acceleration value
    //     int newPosition = currentPosition + camHorizontalAccel;

    //     // Constrain the new position to be within the servo's range
    //     newPosition = constrain(newPosition, 500, 2500);

    //     // Move the servo to the new position
    //     goBildaServo.writeMicroseconds(newPosition);

    //     // Update the current position for the next iteration
    //     currentPosition = newPosition;

    //     // Add a small delay to allow the servo to move smoothly
    //     delay(20);

    // }
    // if (currentPosition <= 500) {
    //     goBildaServo.writeMicroseconds(500);
    // } else if (currentPosition >= 2500) {
    //     goBildaServo.writeMicroseconds(2500);
    // }
}

void setup() {
    Serial0.begin(115200);
    // Allow allocation of all timers
    ESP32PWM::allocateTimer(0);
  
    // 50Hz is standard for servos
    goBildaServo.setPeriodHertz(50);

    // CRITICAL: goBILDA 2000-0025-0002 uses 500us (0 deg) to 2500us (300 deg)
    goBildaServo.attach(servoPWM1, 500, 2500);
    xboxController.begin();
}

void loop() {
    xboxController.onLoop();
    // // You can also write raw microsecond pulse widths for finer control:
    // goBildaServo.writeMicroseconds(500); // Move to 0 degrees, between 500 and 2500 for this servo
    // delay(1000);
    // goBildaServo.writeMicroseconds(1500); // Move to 150 degrees
    // delay(1000);
    // goBildaServo.writeMicroseconds(2500); // Move to 300 degrees
    // delay(1000);

    if (xboxController.isConnected()) {
            int joyStickR = xboxController.xboxNotif.joyRHori;
            // Serial0.print("Joy Stick R: ");
            // Serial0.print(joyStickR);
            controlServoMotor(joyStickR);
            delay(100); // Add a small delay to avoid flooding the serial output when connected
        }
    else {
        Serial0.println("not connected");
        if (xboxController.getCountFailedConnection() > 2) {
            ESP.restart();
        }
        delay(500); // Add a delay to avoid flooding the serial output when not connected
    }

}