#include <SoftwareSerial.h>   //Software Serial Port
#include <Servo.h>

#define RxD 7
#define TxD 6
#define ConnStatus A1

#define DEBUG_ENABLED  1

// ##################################################################################
// ### EDIT THE LINES BELOW TO MATCH YOUR SHIELD NUMBER AND CONNECTION PIN OPTION ###
// ##################################################################################

int shieldPairNumber = 12;

// CAUTION: If ConnStatusSupported = true you MUST NOT use pin A1 otherwise "random" reboots will occur
// CAUTION: If ConnStatusSupported = true you MUST set the PIO[1] switch to A1 (not NC)

boolean ConnStatusSupported = true;   // Set to "true" when digital connection status is available on Arduino pin

// ######################### ##############################

// The following two string variable are used to simplify adaptation of code to different shield pairs

String slaveNameCmd = "\r\n+STNA=Slave";   // This is concatenated with shieldPairNumber later

SoftwareSerial blueToothSerial(RxD,TxD);

// declare servos and pins
Servo servoLeft;
Servo servoRight;

int LServo = 13;
int RServo = 12;

int LReceiver = A0;
int RReceiver = A2 ;
int MReceiver = A1;

void setup()
{
    Serial.begin(9600);
    blueToothSerial.begin(38400);                    // Set Bluetooth module to default baud rate 38400
    
    pinMode(RxD, INPUT);
    pinMode(TxD, OUTPUT);
    pinMode(ConnStatus, INPUT);
    // sensors
    pinMode(LReceiver, INPUT);
    pinMode(RReceiver, INPUT);
    pinMode(MReceiver, INPUT);
  
    // servos
    servoLeft.attach(LServo);
    servoRight.attach(RServo);

    //  Check whether Master and Slave are already connected by polling the ConnStatus pin (A1 on SeeedStudio v1 shield)
    //  This prevents running the full connection setup routine if not necessary.

    if(ConnStatusSupported) Serial.println("Checking Slave-Master connection status.");

    if(ConnStatusSupported && digitalRead(ConnStatus)==1)
    {
        Serial.println("Already connected to Master - remove USB cable if reboot of Master Bluetooth required.");
    }
    else
    {
        Serial.println("Not connected to Master.");
        
        setupBlueToothConnection();   // Set up the local (slave) Bluetooth module

        delay(1000);                  // Wait one second and flush the serial buffers
        Serial.flush();
        blueToothSerial.flush();
    }
}


void loop()
{
    char recvChar;
    int left = digitalRead(LReceiver);
    int middle = digitalRead(MReceiver);
    int right = digitalRead(RReceiver);

    while(1)
    {
        if(blueToothSerial.available())   // Check if there's any data sent from the remote Bluetooth shield
        {
            recvChar = blueToothSerial.read();
            Serial.print(recvChar);
            // manual mode
            if (recvChar == 'w'){
              forward(100);
            }
            if (recvChar == 's'){
              backward(100);
            }
            if (recvChar == 'a'){
              turnLeft(100);
            }
            if (recvChar == 'd'){
              turnRight(100);
            }
            if (recvChar == 'q'){
              stop_car();
            }
            // switch to auto mode
            if (recvChar == 'r'){
                // forward
                if (left == 0 && right == 0) {
                    forward(0);
                }
                // lean right
                if (left == 1 && right == 0) {
                    leanRight(10);
                }
                // lean left
                if (left == 0 && right == 1) {
                    leanLeft(10);
                }
                // stop
                if (left == 1 && right == 1) {
                    stop_car();
                }
            }
        }

        if(Serial.available())            // Check if there's any data sent from the local serial terminal. You can add the other applications here.
        {
            recvChar  = Serial.read();
            Serial.print(recvChar);
            blueToothSerial.print(recvChar);
        }
    }
}
  

void setupBlueToothConnection()
{
    Serial.println("Setting up the local (slave) Bluetooth module.");

    slaveNameCmd += shieldPairNumber;
    slaveNameCmd += "\r\n";

    blueToothSerial.print("\r\n+STWMOD=0\r\n");      // Set the Bluetooth to work in slave mode
    blueToothSerial.print(slaveNameCmd);             // Set the Bluetooth name using slaveNameCmd
    blueToothSerial.print("\r\n+STAUTO=0\r\n");      // Auto-connection should be forbidden here
    blueToothSerial.print("\r\n+STOAUT=1\r\n");      // Permit paired device to connect me
    
    //  print() sets up a transmit/outgoing buffer for the string which is then transmitted via interrupts one character at a time.
    //  This allows the program to keep running, with the transmitting happening in the background.
    //  Serial.flush() does not empty this buffer, instead it pauses the program until all Serial.print()ing is done.
    //  This is useful if there is critical timing mixed in with Serial.print()s.
    //  To clear an "incoming" serial buffer, use while(Serial.available()){Serial.read();}

    blueToothSerial.flush();
    delay(2000);                                     // This delay is required

    blueToothSerial.print("\r\n+INQ=1\r\n");         // Make the slave Bluetooth inquirable
    
    blueToothSerial.flush();
    delay(2000);                                     // This delay is required
    
    Serial.println("The slave bluetooth is inquirable!");
}

// functions
void forward(int t)
{
    servoLeft.writeMicroseconds(1530);
    servoRight.writeMicroseconds(1470);
    delay(t);
}

void backward(int t)
{
    servoLeft.writeMicroseconds(1470);
    servoRight.writeMicroseconds(1530);
    delay(t);
}

void leanRight(int t)
{
    servoLeft.writeMicroseconds(1530);
    servoRight.writeMicroseconds(1500);
    delay(t);
}

void leanLeft(int t)
{
    servoLeft.writeMicroseconds(1500);
    servoRight.writeMicroseconds(1470);
    delay(t);
}

void turnRight(int t)
{
    servoLeft.writeMicroseconds(1530);
    servoRight.writeMicroseconds(1530);
    delay(t);
}

void turnLeft(int t)
{
    servoLeft.writeMicroseconds(1470);
    servoRight.writeMicroseconds(1470);
    delay(t);
}

void stop_car()
{
    servoLeft.writeMicroseconds(1500);
    servoRight.writeMicroseconds(1500);
}
