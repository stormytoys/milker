//command string will be formated as such
//C1,10,2 or C1,10,2
typedef struct {
  bool state;
  uint16_t delayPeriod;
  uint8_t delayPeriodType;
} Command;

const uint8_t delayPeriodTypeMillis   = 1;
const uint8_t delayPeriodTypeSeconds  = 2;
const uint8_t delayPeriodTypeMins     = 3;

//command array defines
Command commands[150];
uint8_t maxCommands = 0; //keeping count of current commands
uint8_t currentCommand = 0;
long currentDelay = 0;
unsigned long previousMillis = 0;

bool processCommands = false;
bool firstRun = false;

const int outputPin =  13;      // the number of the LED pin

String lineBuffer;
char commandType;
int firstSplit = 0;
int secondSplit = 0;

bool bufferState = false;
uint16_t bufferDelay = 0;
uint8_t bufferDelayPeriod = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(outputPin, OUTPUT);
  digitalWrite(outputPin, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  readProcess();

  if(processCommands) {
    if(firstRun) {
      //ok lets start the process off
      runCommand(0);
      firstRun = false;
    }
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= currentDelay) {
      previousMillis = currentMillis;
      if(++currentCommand > maxCommands) {
        currentCommand = 0;
      }
      runCommand(currentCommand);
    }
  }
}

void runCommand(uint8_t pos) {
  digitalWrite(outputPin, commands[pos].state > 0?HIGH:LOW);
  if(commands[pos].delayPeriodType == delayPeriodTypeMillis) {
    currentDelay = (long)commands[pos].delayPeriod;
  } else if(commands[pos].delayPeriodType == delayPeriodTypeSeconds) {
    currentDelay = (long)commands[pos].delayPeriod * 1000;
  } else if(commands[pos].delayPeriodType == delayPeriodTypeMins) {
    currentDelay = (long)commands[pos].delayPeriod * 60000;
  }
}

bool readProcess() {
  if (Serial.available() > 0) {
  //sending an I char will start the input process
    if(Serial.find('I')) {
      bool inputLoop = true;
      digitalWrite(outputPin, LOW); //shutoffOutput
      maxCommands = 0; //max command reset
      currentCommand = 0;
      previousMillis = 0;
  
      commands[maxCommands].delayPeriodType = 0;
      Serial.println("R");
  
      while(inputLoop) {
        //ok lets read in the commands
        if (Serial.available() > 0) {
          lineBuffer = Serial.readStringUntil('\n');
          Serial.read();
          
          if(lineBuffer.length() > 0) {
            
            commandType = lineBuffer.charAt(0);
            bufferState = false;
            bufferDelay = 0;
            bufferDelayPeriod = 0;
            firstSplit = 0;
            secondSplit = 0;
            
            if(commandType == 'C') {
              //C1,200,2
              firstSplit = lineBuffer.indexOf(',');
              bufferState = lineBuffer.substring(1,firstSplit).toInt()>0?true:false;
              secondSplit = lineBuffer.indexOf(',', firstSplit+1);
              bufferDelay = (uint16_t)lineBuffer.substring(firstSplit+1, secondSplit).toInt();
              bufferDelayPeriod = (uint8_t)lineBuffer.substring(secondSplit+1).toInt();

              if(bufferDelay > 0 && bufferDelayPeriod > 0) {
                if(commands[0].delayPeriodType != 0) {
                  maxCommands++;
                }
                commands[maxCommands].state = bufferState;
                commands[maxCommands].delayPeriod = bufferDelay;
                commands[maxCommands].delayPeriodType = bufferDelayPeriod;
                Serial.print("GOT: ");
                Serial.print(commands[maxCommands].state?1:0, DEC);
                Serial.print(",  ");
                Serial.print(commands[maxCommands].delayPeriod, DEC);
                Serial.print(",  ");
                Serial.print(commands[maxCommands].delayPeriodType, DEC);
                Serial.println("");
              }
            }

            if(commandType == 'F') {
              inputLoop = false; //close off the input loop
              Serial.println("Going to start");
            }
          }
          
        }
      }
      firstRun = true;
      processCommands = true;
      return true;
    }
  }

  return false;
}

