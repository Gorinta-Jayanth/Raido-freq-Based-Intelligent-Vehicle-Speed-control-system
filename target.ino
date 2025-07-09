/**
  @file    nfc_p2p_target.ino
  @brief   NFC P2P Target with Default Motor Speed and Calibration

  This code sets the motors to run at a default speed of 80 upon power-up.
  It includes a calibration factor to synchronize the motor speeds from speeds 10 to 90.
  The calibration is not applied at speed 100, as the motors are already synchronized.
*/

#include "nfc.h"  // Include the NFC library

/** Define an NFC class */
NFC_Module nfc;

/** Define RX and TX buffers, and length variable */
u8 tx_buf[50];
u8 tx_len;
u8 rx_buf[50];
u8 rx_len;

/** Motor control pins */
#define MOTOR_A_PIN1 5   // PWM-capable pin for Motor A
#define MOTOR_A_PIN2 6   // PWM-capable pin for Motor A
#define MOTOR_B_PIN1 9   // PWM-capable pin for Motor B
#define MOTOR_B_PIN2 10  // PWM-capable pin for Motor B

/** Default motor speed */
int motorSpeed = 80; // Default speed before receiving any input

/** Calibration offset for Motor B in the speed range 10 to 90 */
#define CALIBRATION_OFFSET 5.05  // Adjust this value based on testing

void setup(void)
{
  Serial.begin(115200);
  nfc.begin();
  Serial.println("P2P Target Demo with Default Motor Speed and Calibration!");

  uint32_t versiondata = nfc.get_version();
  if (!versiondata) {
    Serial.println("Didn't find PN53x board");
    while (1); // halt
  }

  // Print chip and firmware information
  Serial.print("Found chip PN5"); Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata >> 8) & 0xFF, DEC);

  /** Set normal mode, and disable SAM */
  nfc.SAMConfiguration();

  // Initialize motor pins
  pinMode(MOTOR_A_PIN1, OUTPUT);
  pinMode(MOTOR_A_PIN2, OUTPUT);
  pinMode(MOTOR_B_PIN1, OUTPUT);
  pinMode(MOTOR_B_PIN2, OUTPUT);

  // Set motors to default speed with calibration
  setMotorSpeed(motorSpeed);
}

void loop(void)
{
  // Clear the receive buffer
  memset(rx_buf, 0, sizeof(rx_buf));

  /** Device is configured as Target */
  if (nfc.P2PTargetInit()) {
    if (nfc.P2PTargetTxRx(tx_buf, tx_len, rx_buf, &rx_len)) {
      // Ensure the received data is null-terminated
      rx_buf[rx_len] = '\0';

      Serial.print("Data Received: ");
      Serial.write(rx_buf, rx_len);
      Serial.println();

      // Construct String from rx_buf using rx_len
      String receivedString = String((char*)rx_buf).substring(0, rx_len);

      // Convert received data to integer
      int receivedSpeed = receivedString.toInt();

      Serial.print("Converted Speed: ");
      Serial.println(receivedSpeed);

      // Clamp the speed value between 0 and 100
      receivedSpeed = constrain(receivedSpeed, 0, 100);

      // Update motor speed with the received value
      motorSpeed = receivedSpeed;

      // Set motor speed based on received data
      setMotorSpeed(motorSpeed);

      // Add a delay to avoid continuous reception of the same data
      delay(500);
    }
  }
}

void setMotorSpeed(int speed)
{
  // Map speed from 0-100 to PWM range 0-255
  int pwmValue = map(speed, 0, 100, 0, 255);
  int pwmValueA = pwmValue; // PWM value for Motor A
  int pwmValueB = pwmValue; // PWM value for Motor B

  // Apply calibration offset to Motor B in the speed range 10 to 90
  if (speed >= 10 && speed <= 90) {
    // Adjust Motor B's PWM value with calibration offset
    pwmValueB = pwmValueB - CALIBRATION_OFFSET;
    // Ensure pwmValueB does not exceed 255
    pwmValueB = constrain(pwmValueB, 0, 255);
  }

  // Set Motor A speed
  analogWrite(MOTOR_A_PIN1, pwmValueA);
  digitalWrite(MOTOR_A_PIN2, LOW); // Ensure the opposite pin is LOW

  // Set Motor B speed
  analogWrite(MOTOR_B_PIN1, pwmValueB);
  digitalWrite(MOTOR_B_PIN2, LOW); // Ensure the opposite pin is LOW

  // Debugging output
  Serial.print("Motor Speed Set To: ");
  Serial.println(speed);
  Serial.print("Motor A PWM Value: ");
  Serial.println(pwmValueA);
  Serial.print("Motor B PWM Value: ");
  Serial.println(pwmValueB);
}
