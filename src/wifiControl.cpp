#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "library verizon";
const char* password = "knit7-fumes-dab";

const int udpPort = 4210;               // port number that needs to be set on both sides
WiFiUDP udp;                            // an object that manages incoming UDP packets
char incomingPacket[16];                

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
    Serial0.begin(115200);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial0.print(".");             // prints dots while connecting
    }

    Serial0.println("Connected!");
    Serial0.print("My IP address is: ");
    Serial0.println(WiFi.localIP());    //IP address of the ESP32

    udp.begin(udpPort);                 // start listening on that port

    pinMode(PWM_A, OUTPUT);
    pinMode(AIN_1, OUTPUT);
    pinMode(AIN_2, OUTPUT);

    pinMode(PWM_B, OUTPUT);
    pinMode(BIN_1, OUTPUT);
    pinMode(BIN_2, OUTPUT);
    
}

void loop() {
    int packetSize = udp.parsePacket();   // checks: did anything arrive?

    if (packetSize) {                     // packetSize is 0 if nothing came in
        int len = udp.read(incomingPacket, sizeof(incomingPacket) - 1);
        if (len > 0) {
            incomingPacket[len] = 0;      // marks the end of the text (like a period at the end of a sentence)
        }
        Serial0.print("Received: ");
        Serial0.println(incomingPacket);

        switch (incomingPacket[0]) {        // checks the first character of the text
            case 'F':
                Serial0.println("Forward");
                controlMotor(1023, 0, 32768); // full forward
                delay(500);
                controlMotor(0, 0, 32768); // stop
                break;
            case 'B':
                Serial0.println("Backward");
                controlMotor(0, 1023, 32768); // full backward
                delay(500);
                controlMotor(0, 0, 32768); // stop
                break;
            case 'L':
                Serial0.println("Left");
                break;
            case 'R':
                Serial0.println("Right");
                break;
            default:
                Serial0.println("Unknown command");
                break;
        }
    }
    delay(200);
}