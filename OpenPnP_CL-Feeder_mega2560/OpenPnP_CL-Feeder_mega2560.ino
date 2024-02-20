//---------------------------------
// Pneumatic valve controller for OpenPnP
//
// Modified by cronos_sv
// Date : 2024/Jan/29_1805
//
// target : Arduino Mega2560
//---------------------------------
#define MaxPinCount 32
//#define CommandDebug

 // Define power control pins
const uint8_t Power_Control_Pin = A15;  // Not used at this time.

const uint16_t ON_TIME = 80;

/*
const uint8_t Motor_Control_Pin[MaxPinCount] = {
  49, 48, 47, 46, 45, 44, 43, 36,
  37, 38, 39, 40, 41, 42, 29, 30,
  31, 32, 33, 34, 35, 22, 23, 24,
  25, 26, 27, 28, 7, 6, 5, 4
};
*/

const uint8_t Motor_Control_Pin[MaxPinCount] = {
  34, 35, 36, 37, 38, 39, 40, 41, 
  42, 49, 48, 47, 46, 45, 44, 43, 
  33, 32, 31, 30, 29, 28, 27, 26,
  25, 24, 23, 22, 4, 5, 6, 7
};

// Variables
uint8_t pinValue = 0;     // Store Pin value
uint8_t pulseValue = 0;   // Store impulse value
uint16_t cmdValue = 0;    // Store command value
uint8_t rxCount = 0;      // Used for serial port receiving count
char rxBuffer[200] = {0}; // Store data received by serial port

void setup()
{
  Serial.begin(115200);                   // Initialize serial port, baud rate 9600
  pinMode(Power_Control_Pin, OUTPUT);   // Set the power control pin to output mode
  digitalWrite(Power_Control_Pin, LOW); // Set the power control pin output low level
  for (uint8_t i = 0; i < MaxPinCount; i++)      // 
  {
    pinMode(Motor_Control_Pin[i], OUTPUT);         // Set the motor control pin to output mode
    digitalWrite(Motor_Control_Pin[i], LOW);       // Set the motor control pin output low level
  }

  delay(1500);  // Prevent feeder malfunction at startup.
}

void loop()
{
  while (Serial.available()) // Cycle until no data is readable on the serial port
  {
    rxBuffer[rxCount] = Serial.read(); // Read one bit data
    if (rxBuffer[rxCount] == '\n')     // If the terminator is received
    {
      rxBuffer[rxCount] = 0;    // Set packet Terminator
      Serial.println(rxBuffer); // Print received data
      Unpack_Data();            // Parse data
      rxCount = 0;              // Restart receiving data
    }
    else                //
      rxCount++;      // 
    if (rxCount >= 200) // If the received data exceeds the size of the serial port buffer array
      rxCount = 0;    // Restart receiving
  }
}

void Unpack_Data(void)
{
  char *Tail = NULL, *Head = NULL;            // Pointer to store parsing data
  if ((Head = strchr(rxBuffer, 'M')) != NULL) // If M is found
  {
    if ((Tail = strchr(Head, ' ')) != NULL) //If a space character is found
    {
      *Tail = 0;                 //Truncate string
      cmdValue = atoi(Head + 1); // 

      if (cmdValue == 610)       // 610 mean Winder-motor Main Power Control
      {
        if (strstr(Tail + 1, "S1") != NULL) // If S1 is searched
        {
          digitalWrite(Power_Control_Pin, HIGH); // Power on
          Serial.print(F("ok "));
#ifdef CommandDebug
          Serial.print(1);                       //
#endif
        }
        else if (strstr(Tail + 1, "S0") != NULL) // If S0 is searched
        {
          digitalWrite(Power_Control_Pin, LOW); // Turn off the power
#ifdef CommandDebug
          Serial.print(0);                      // 
#endif
            for (uint8_t i = 0; i < MaxPinCount; i++)      // 
            {
              digitalWrite(Motor_Control_Pin[i], LOW);       // Set the motor control pin output low level
            }
            Serial.print(F("ok "));
        } else {
          Serial.print(F("error "));
        }
      }

      else if (cmdValue == 600) // 600 mean Servo-pin control of any port
      {
        Head = strchr(Tail + 1, 'N');             // Find N
        Tail = strchr(Head, ' ');                 // Find spaces
        *Tail = 0;                                // Truncate string
        pinValue = atoi(Head + 1);                // Get pin number

        pulseValue = 0;
        Head = strchr(Tail + 1, 'S');             // Find S
        pulseValue = atoi(Head + 1);              // Get propulsion value

        if(pinValue < MaxPinCount)
        {
#ifdef CommandDebug
          Serial.print(cmdValue);    // 
          Serial.print(",");         // 
          Serial.print(pinValue);                   // 
          Serial.print(",");                        // 
          Serial.print(pulseValue);                 // 
#endif
          digitalWrite(Motor_Control_Pin[pinValue], pulseValue);
          delay(ON_TIME);      // The time required for the feeder mechanism to move.
          Serial.print(F("ok "));
        } else
        {
          Serial.print(F("error "));
#ifdef CommandDebug
          Serial.print("###ERR : M");
          Serial.print(cmdValue);
          Serial.print(" Invalid Port Number");
          Serial.println(); //
          Serial.print("         Valid Port Number : 0 to ");
          Serial.print(MaxPinCount-1);
#endif
        }
      }
      Serial.println(); //
    } else
    {
      cmdValue = atoi(Head + 1); // 
      if (cmdValue == 114)            // 114 is respond with the fixed string "X:0.0"
      {
        Serial.print(F("X:0.0 "));
        Serial.println();
        Serial.print(F("ok "));
      }
       else if (cmdValue == 115)            // 115 mean Inform System Information to OpenPnP
      {
        Serial.print(F("FIRMWARE_NAME: TSW Electronics OpenPnP CL-Feeder_mega2560 NUMBER_OF_FEEDERS: 0_to_"));
        Serial.print(MaxPinCount-1);
        Serial.print(F(" ELECTRONICS: TSW Electronics CL-Feeder Driver for mega2560 FIRMWARE_VERSION: 24.2.20_1724 "));
        Serial.println();
        Serial.print(F("ok "));
      }
      Serial.println(); //
    }
  }
}
