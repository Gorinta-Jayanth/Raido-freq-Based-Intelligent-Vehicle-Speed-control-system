/**
  @file    nfc_p2p_initiator.ino
  @brief   Example of Peer-to-Peer communication for NFC_MODULE with matrix keypad and I2C LCD display.
*/

/** Include libraries */
#include "nfc.h"
#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h> // Include the I2C LCD library

/** Define an NFC class */
NFC_Module nfc;
/** Define RX and TX buffers, and length variables */
u8 tx_buf[50];
u8 tx_len;
u8 rx_buf[50];
u8 rx_len;

/** Keypad setup */
const byte ROWS = 4; // Four rows
const byte COLS = 4; // Four columns
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {2, 3, 4, 5}; // Connect to the row pinouts of the keypad
byte colPins[COLS] = {6, 7, 8, 9}; // Connect to the column pinouts of the keypad
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

/** LCD setup */
LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD I2C address to 0x27 and specify 16 columns and 2 rows

String inputString = "";
String confirmedString = "10"; // Default number to print continuously
const String validNumbers[] = {"0", "10", "20", "30", "40", "50", "60", "70", "80", "90", "100"};

void setup(void)
{
  Serial.begin(115200);
  /** NFC initialization */
  nfc.begin();
  Serial.println("P2P Initiator Demo BY ELECHOUSE!");
  
  uint32_t versiondata = nfc.get_version();
  if (!versiondata) {
    Serial.println("Didn't find PN53x board");
    while (1); // Halt
  }
  
  // Got OK data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata >> 24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata >> 16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata >> 8) & 0xFF, DEC);
  
  /** Set normal mode, and disable SAM */
  nfc.SAMConfiguration();

  /** Initialize the LCD and display the default confirmed value */
  lcd.init();      // Initialize the LCD
  lcd.backlight(); // Turn on the backlight
  lcd.clear();
  lcd.print("  Current speed:");
  displayCenteredValue(confirmedString);
}

void loop(void)
{
  char key = keypad.getKey();
  
  if (key) {
    if (key == '#') {
      // Validate and confirm the input string
      bool isValid = false;
      for (int i = 0; i < 11; i++) {
        if (inputString == validNumbers[i]) {
          isValid = true;
          break;
        }
      }
      if (isValid) {
        confirmedString = inputString;
        inputString = "";

        /** Display the confirmed value on the LCD */
        lcd.clear();
        lcd.print(" Current speed:");
        displayCenteredValue(confirmedString);

      } else {
        Serial.println("Invalid input. Please enter a valid number.");
        inputString = "";

        /** Display invalid input message */
        lcd.clear();
        lcd.print("Invalid Input");
      }
    }
    else if (key == '*') {
      // Backspace: remove the last character
      if (inputString.length() > 0) {
        inputString.remove(inputString.length() - 1);
      }

      /** Update the LCD with current input */
      lcd.clear();
      lcd.print("Input:");
      lcd.setCursor(0, 1);
      lcd.print(inputString);

    }
    else {
      // Append the pressed key to the input string
      inputString += key;

      /** Update the LCD with current input */
      lcd.clear();
      lcd.print("Input:");
      lcd.setCursor(0, 1);
      lcd.print(inputString);
    }

    Serial.print("Input: "); Serial.println(inputString);
    Serial.print("Confirmed: "); Serial.println(confirmedString);
  }

  // Convert the confirmed string to a character array and send via NFC
  confirmedString.toCharArray((char*)tx_buf, 50);
  tx_len = confirmedString.length();

  if (nfc.P2PInitiatorInit()) {
    if (nfc.P2PInitiatorTxRx(tx_buf, tx_len, rx_buf, &rx_len)) {
      /** Send and receive successfully */
      Serial.print("Vehicle sensed");
      Serial.write(rx_buf, rx_len);
      Serial.println();
    }
    Serial.println();
  }
}

void displayCenteredValue(String value) {
  int length = value.length();
  int padding = (16 - length) / 2; // Calculate padding to center the text
  lcd.setCursor(padding, 1); // Set cursor to the center line with calculated padding
  lcd.print(value);
}
