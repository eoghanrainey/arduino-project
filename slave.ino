
#include <Servo.h>

Servo myservo;  // create servo object to control a servo


#define MOA 2
#define MOB 3

void setup() {
  // initialize serial:
  Serial.begin(9600);
  Serial1.begin(9600);
  pinMode(MOA,OUTPUT);
  pinMode(MOB,OUTPUT);
  pinMode(LED_BUILTIN,OUTPUT);
  myservo.attach(4);  

  myservo.write(150);
  
}

void loop() {
  // if there's any serial available, read it:
  while (Serial1.available() > 0) {

    if (Serial1.read() == '%') {
       // Turn on motor(led)
       Serial.println("Servo at 30 degree");
        myservo.write(30);
        digitalWrite(LED_BUILTIN ,HIGH);
        digitalWrite(MOA ,HIGH);
        digitalWrite(MOB ,LOW);
        delay(5000);
        digitalWrite(LED_BUILTIN ,LOW);
        digitalWrite(MOA ,LOW);
        digitalWrite(MOB ,LOW);
        myservo.write(150);
        Serial.println("Servo at 150 degree");
    }
  }

    while (Serial.available() > 0) {

    if (Serial.read() == '%') {
       // Turn on motor(led)
       Serial.println("Servo at 30 degree");
        myservo.write(30);
        digitalWrite(LED_BUILTIN ,HIGH);
        digitalWrite(MOA ,HIGH);
        digitalWrite(MOB ,LOW);
        delay(5000);
        digitalWrite(LED_BUILTIN ,LOW);
        digitalWrite(MOA ,LOW);
        digitalWrite(MOB ,LOW);
        myservo.write(150);
        Serial.println("Servo at 150 degree");
    }
  }
}
