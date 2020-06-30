
*/
#include <Arduino.h>
// include the library code:
#include <LiquidCrystal.h>
// include the Keypad code:
#include <Keypad.h>
#if ARDUINO_ARCH_SAMD
#include <FlashAsEEPROM.h>
#else
#include<EEPROM.h>
#endif

#if ARDUINO_ARCH_SAMD
//// Serial2 pin and pad definitions (in Arduino files Variant.h & Variant.cpp)
//#define PIN_SERIAL2_RX       (1ul)                // Pin description number for PIO_SERCOM on D1
//#define PIN_SERIAL2_TX       (0ul)                // Pin description number for PIO_SERCOM on D0
//#define PAD_SERIAL2_TX       (UART_TX_PAD_0)      // SERCOM pad 0 TX
//#define PAD_SERIAL2_RX       (SERCOM_RX_PAD_1)    // SERCOM pad 1 RX
//Uart Bluetooth(&sercom3, PIN_SERIAL2_RX, PIN_SERIAL2_TX, PAD_SERIAL2_RX, PAD_SERIAL2_TX);
#else
#include <SoftwareSerial.h>
SoftwareSerial Bluetooth(6, 7);

#endif

// Function prototypes
void lcdClearLine(char line);

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 5, en = 4, d4 = 10, d5 = 11, d6 = 12;
const int d7 = A6;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

#define MOA 2
#define MOB 3



#define USERPIN_LEN  4
#define MASTERPIN_LEN 6

#define USERPIN_MODE 0 
#define MASTERPIN_MODE 1
int addr=0;

// intialise the keypad pins
const byte rows = 4; //four rows
const byte cols = 4; //three columns
char keys[rows][cols] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'#','0','*','D'}
};
byte rowPins[rows] = {A0, A5, 8, A4}; //connect to the row pinouts of the keypad
byte colPins[cols] = {9, A3, A2, A1}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, rows, cols );

String enternedPIN = "";
String enternedBluetoothPIN = "";
String masterPIN = "123456";
String userPIN = "1234";

char pinNumMode = USERPIN_MODE;

// used for avoid triggering multi input when user holds down 
// the key for a while
bool keyDepressed = true; 

bool saveUserPIN = false;

// reads the userPIN from EEPROM at startup
void getPINfromEprom(){
  userPIN = "";
  for (int i = 0 ; i< 4; i++){
  char val = EEPROM.read(addr + i); 
//Serial.println(val);
  userPIN = userPIN +  val;
  }  
}

// save the pin to EEPROM for reading the newpin at next startup
void putPINtoEprom(String pin){
  for (int i = 0 ; i< 4; i++){
    EEPROM.write(addr + i, pin[i]);  
  }  
#if ARDUINO_ARCH_SAMD
  EEPROM.commit();
#endif
}

void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("Program starting");
  //initialise lcd diaplay
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);

  // show ready message to the user
  lcd.print("EASY PASS     ");
  pinMode(MOA,OUTPUT);
  pinMode(MOB,OUTPUT);

  // initialise bluetooth serial port
  Serial1.begin(9600);
  //Serial.println("Waiting for command...");
  Serial1.println("connected");
  //Serial.println("Entering main loop");

  // save the userPIN to EEPROM for the very first time only
#if ARDUINO_ARCH_SAMD  
  if (!EEPROM.isValid()) {
    Serial.println("EEPROM is empty, so writing the pin");
    putPINtoEprom(userPIN);
  }
#endif 

  // get the userPIN from EEPROM for comparing with the user entered value
  //putPINtoEprom("1234");
  getPINfromEprom();
  Serial.print("Current PIN: ");
  Serial.println(userPIN);
}

void loop() {
  //Serial.println("In loop");
//  delay(1000);
  lcd.setCursor(0, 1);
 //lcd.print(millis() / 1000);
  char key = keypad.getKey();
  
  if (key && keyDepressed){   // enter if user pressed a key
    keyDepressed = false;
    Serial.println(key);

    // Append to PIN if 0 to 9
   if (key >= '0' && key <= '9'){  // append to string for comparing later
     enternedPIN = enternedPIN + key;
    }
    else{
      enternedPIN = "";
      saveUserPIN = false;
      
    if (key == 'A'){   // enter the master PIN entering mode
        // Enable master pin input mode to change the user pin
        pinNumMode = MASTERPIN_MODE;
        lcdClearLine(0);
        lcd.setCursor(0, 0);
        lcd.print("IN MASTER PASS");
        lcdClearLine(1);
        Serial.println("Entering master pin mode");
      }else if (key == 'C'){  // Cancel the current operation and enter ready mode
        lcdClearLine(0);
        lcd.setCursor(0, 0);
        lcd.print("EASY PASS     ");
        lcdClearLine(1);
        pinNumMode = USERPIN_MODE;
        Serial.println("Reset user pin mode");
      }
    }

    // show * in LCD instead of PIN
    if (enternedPIN.length() > 0){
      lcdClearLine(1);
      lcd.setCursor(0, 1);
      Serial.print("PASS:");

      // show * character for each entered digit for security reason
      for (int i =0; i<enternedPIN.length();i++){
        lcd.print("*");
        Serial.print(enternedPIN[i]);
      }
      Serial.println("");
    }

    // Enter if 4 digit pin is input by the user
    if (enternedPIN.length() == USERPIN_LEN && pinNumMode == USERPIN_MODE){
      if (saveUserPIN){  // enter if the userPIN entered while the userPIN save mode after the master pin validation
        // save the userPIN
        userPIN = enternedPIN;
        saveUserPIN = false;
        Serial.println("new pin saved");
        Serial.println(userPIN);

        // write EEPROM pin to storage
        putPINtoEprom(userPIN);
        lcdClearLine(1);
        lcd.setCursor(0, 1);
        lcd.print("USER PIN SAVED");
        delay(2500);
        lcdClearLine(0);
        lcdClearLine(1);
        lcd.setCursor(0, 0);
        lcd.print("EASY PASS     ");
        lcdClearLine(1);
        pinNumMode = USERPIN_MODE;
        enternedPIN = "";

        
      }
      else{
        // check whether entered pin is correct when the user entered pin in ready state
        if (enternedPIN == userPIN){
          // only enter if pin matched
          enternedPIN = "";
          Serial.println("Entered pin matched the userPIN");
          lcdClearLine(1);
          lcd.setCursor(0, 1);
          lcd.print("PIN SUCCESS");

          Serial.print("%");
          Serial1.print("%");
          digitalWrite(MOA ,HIGH);
          digitalWrite(MOB ,LOW);
          delay(5000);
          digitalWrite(MOA ,LOW);
          digitalWrite(MOB ,LOW);

          lcdClearLine(1);
          
        }else{
          // enter if pin does not match
          Serial.println("Entered pin does not match");
          lcdClearLine(1);
          lcd.setCursor(0, 1);
          lcd.print("PIN FAILED");
        }
      }
      enternedPIN = "";
      saveUserPIN = false;
    }

    // Enter if 6 digit master pin is input after pressing 'A' and put in master pin mode
   if (enternedPIN.length() == MASTERPIN_LEN && pinNumMode == MASTERPIN_MODE){
      if (enternedPIN == masterPIN){
        // enter if the masterpin matches the pin entered by the user
        enternedPIN = "";
        pinNumMode = USERPIN_MODE;
        Serial.println("Entered master pin matched the userPIN");
        lcdClearLine(0);
        lcd.setCursor(0, 0);
        lcd.print("ENTER USER PIN");
        lcdClearLine(1);
        saveUserPIN = true;

      }
      
      else{
        // enter if the master pin entered is wrong
        lcdClearLine(1);
        lcd.setCursor(0, 1);
        lcd.print("MASTER PIN FAIL ");
        Serial.println("Entered master pin matched the userPIN");
      }
      // clear entered PIN string for next PIN
      enternedPIN="";
    }

    // Delay for reducing debouncing
    delay(250);
  }
  else{
    // If no key pressed, its depressed and enable the keypad input
    keyDepressed = true;
  }

   if(Serial1.available())
   { //enter if bluetooth data is received from mobile application
     char bluetoothData = Serial1.read();
     if (bluetoothData == '$'){
      // enter if delimiter is received
       if (enternedBluetoothPIN == userPIN){
        // only enter if the pin received through bluetooth matched the userPIN
        Serial.println("BluetoothPIN matched");
        Serial1.println("PIN SUCCESS");

        Serial.print("%");
        Serial1.print("%");
        
        digitalWrite(MOA ,HIGH);
        digitalWrite(MOB ,LOW);
        delay(5000);
        digitalWrite(MOA ,LOW);
        digitalWrite(MOB ,LOW);

        lcdClearLine(1);
       }
       else{
        // enter if the bluetooth pin is wrong
        Serial.println("BluetoothPIN failed");
        Serial1.println("PIN FAIL");
       }
       enternedBluetoothPIN = "";
     }else{
      // append the enternedBluetoothPIN with each character received through bluetooth until a delimiter '$' is send 
       enternedBluetoothPIN = enternedBluetoothPIN + bluetoothData;
       Serial.print("BPASS:");
       Serial.println(enternedBluetoothPIN);

       Serial1.print("BPASS:");
       Serial1.println(enternedBluetoothPIN);
       
     }
   }
}

// clear the corresponding line (0  - LINE1 or 1 - LINE2)
void lcdClearLine(char line){
  lcd.setCursor(0, line);
  lcd.print("                ");
}


   
