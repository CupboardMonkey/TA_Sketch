//Use a modified version of Ken Sherriff's IR library
#include <IRremote.h>
#include <SoftwareSerial.h>

//SoftwareSerial allows a definable serial for RX and TX
SoftwareSerial TinySerial(9, 10);

int RECV_PIN = 2;
int LED_OUT = 3;
int STATUS_PIN = 13;

IRrecv irrecv(RECV_PIN);
IRsend irsend;

decode_results results;
unsigned int* channelCode;

char input;
int gotCode = 0;

// Storage for the recorded code
//int codeType = -1; // The type of code
//unsigned long codeValue; // The code value if not raw
unsigned int rawCodes[RAWBUF]; // The durations if raw
int codeLen; // The length of the code

void setup()
{
  TinySerial.begin(9600);
  pinMode(STATUS_PIN, OUTPUT);
  pinMode(LED_OUT, OUTPUT);
}

void loop() {
  //Get or reset the input
  if(TinySerial.available() > 0) {
    input = TinySerial.read(); 
  } else {
    input = 5;  
  }

if (input == '0') {
  
    gotCode = 0;
    irrecv.enableIRIn();
    //Set status to high to indicate listening
    digitalWrite(STATUS_PIN, HIGH);
    
    while(gotCode == 0) {
      if (irrecv.decode(&results)) {
        storeCode(&results);
        //Set status to low to indicate stored
        digitalWrite(STATUS_PIN, LOW);
        gotCode = 1;
      }
    }
  } else if (input == '1') {
    sendCode();
  } 
}


//Ken Sherriff library instruction to store code
void storeCode(decode_results *results) {
  codeLen = results->rawlen - 1;
  // To store raw codes:
  // Drop first value (gap)
  // Convert from ticks to microseconds
  // Tweak marks shorter, and spaces longer to cancel out IR receiver distortion

  for (int i = 1; i <= codeLen; i++) {

    if (i % 2) {
      // Mark
      rawCodes[i - 1] = results->rawbuf[i]*USECPERTICK - MARK_EXCESS;
    } 
    else {
      // Space
      rawCodes[i - 1] = results->rawbuf[i]*USECPERTICK + MARK_EXCESS;
    }
  }
  
  //Send code to application
  sendIntArray(codeLen, rawCodes);
}

void sendCode() {

  //Receive the code and store it to send
  int capacity = receiveInt();
  unsigned int returnArray[100];
  for(int i = 0; i < capacity; i++) {
    returnArray[i] = receiveInt();
  } 

//Ken Sherriff library instruction to send code
  irsend.sendRaw(returnArray, capacity, 38);
}

void sendIntArray(int capacity, unsigned int array[]) {

  //Send the size to tell the application how many to listen for
  sendInt(capacity);
  for(int i = 0; i < capacity; i++) {
    sendInt(array[i]); 
  }
  
}

void sendInt(unsigned int i) {
  //Send the integer followed by an 'x' to signal end of number
  TinySerial.print(i);
  TinySerial.print('x');
}

unsigned int receiveInt() {
  char readChar = 0;
  unsigned int number = 0;
  boolean condition = true;

  while(condition) { 
    readChar = TinySerial.read();
    //If end of number then return number
    if(readChar == 'x') {
      condition = false;
      return number;
    } 
    //Else multiply current number by 10 and add unit
    else {
      int v = readChar - '0';
      if(v >= 0 && v <= 9) {
        number = number * 10;
        number = number + v;
      }
    }
  }  
}
