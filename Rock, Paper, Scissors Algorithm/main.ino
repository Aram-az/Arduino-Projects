/*
  Configure the pins for the buttons and LEDS here
  -----------------------------------------------------
*/
const int BTN1 = 3;  // Button closest to header pins
const int BTN2 = 4;  // Button fartheest from LEDs
const int BTN3 = 5;  // Button farthest from header pins
const int BTN4 = 2;  // Button closest to LEDs

const int LED1 = 12;  // Leftmost LED
const int LED2 = 11;
const int LED3 = 10;
const int LED4 = 9;
const int LED5 = 8;  // Rightmost LED
/*
  ---------------------------------------------------
*/

const int BTN_PRESSED_SIGNAL = LOW;

/*
  Ordered such that index of LED in LEDS is their bit position in LED double bit pattern
  E.g. for Sample Pattern: 1101010000
  pattern:        11    01    01    00    00
  Bit Pos:         4     3     2     1     0
  Assigned LED: LED1  LED2  LED3  LED4  LED5
*/
const int LEDS[] = { LED5, LED4, LED3, LED2, LED1 };
const int LED_COUNT = 5;


/*
  LED Double Bit Pattern uses 2 bits for each LED:
  00 -> off
  01 -> blinking
  11 -> on

  You can combine the enumerated types to make a simple LED pattern, e.g.
  LED1 is on, but LED2 and LED3 will be blinking
  = LED1_ON | LED2_BLINKING | LED3_BLINKIng
*/
enum LEDDoubleBitPattern {
  LED_OFF=0b000,
  LED_BLINKING=0b001,
  LED_ON=0b011,

  /*
  LEDs are ordered left to right, so bit pattern:
  Case: LED1 stable, LED2 and LED3 blinking
  Sample pattern: 11   01   01   00   00
  Bit Pos:        4    3    2    1    0
  Assigned LED:   LED1 LED2 LED3 LED4 LED5
  */

  LED5_OFF      = LED_OFF      << 0,
  LED5_BLINKING = LED_BLINKING << 0,
  LED5_ON       = LED_ON       << 0,
  
  // left shift by 2 as each LED has 2 bits asssigned to it
  LED4_OFF      = LED_OFF      << 2,
  LED4_BLINKING = LED_BLINKING << 2,
  LED4_ON       = LED_ON       << 2,

  LED3_OFF      = LED_OFF      << 4,
  LED3_BLINKING = LED_BLINKING << 4,
  LED3_ON       = LED_ON       << 4,

  LED2_OFF      = LED_OFF      << 6,
  LED2_BLINKING = LED_BLINKING << 6,
  LED2_ON       = LED_ON       << 6,

  LED1_OFF      = LED_OFF      << 8,
  LED1_BLINKING = LED_BLINKING << 8,
  LED1_ON       = LED_ON       << 8,

  ALL_LEDS_ON = 0b01111111111,
  ALL_LEDS_BLINKING = 0b0101010101,
};

/*
  Mapping of buttons to the round
*/
const int TURN_BTN = BTN3;   // Start the current turn.
const int ROCK_BTN = BTN4;   // Opponent selects rock.
const int PAPER_BTN = BTN1;  // Opponent selects paper.
const int SCR_BTN = BTN2;    // Opponent selects scissors.
int RPS_BTNS[] = { ROCK_BTN, PAPER_BTN, SCR_BTN};
int RPS_BTN_COUNT = 3;

/*
  Set of system states for the LED
*/
enum SystemLEDState {
  WaitingForRound = ALL_LEDS_BLINKING,
  WaitingForOpponent = LED5_BLINKING,
  WaitingForArduino = LED1_BLINKING,
  ArduinoWonRound = LED1_ON,
  OpponentWonRound = LED5_ON,
  Draw = LED1_ON | LED5_ON,
  OutOfRounds = LED_OFF,
  ShowingRPSRock = LED1_ON | LED2_ON ,
  ShowingRPSPaper = LED3_ON,
  ShowingRPSScissors = LED4_ON | LED5_ON,
};

/*
  Enumerated type for the round results.
*/
enum RoundResult {
  Round_None=0,
  Round_Lost=1,
  Round_Draw=2,
  Round_Won=3
};


/*
  Enumerated type for the rock paper scissors choices
*/
enum RPSChoice {
  RPS_None = -1,
  RPS_Rock = 0,
  RPS_Paper = 1,
  RPS_Scissors = 2
};

/*
  Global variables to manage the the rounds
  MAX_ROUNDS : Maximum number of rounds Arduino can play before it has to be reset, in essence how many rounds it can remember
  roundsPlayed : The number of rounds played by the Arduino
  arduinoChoices : The set of RPS Choices made by the Arduino in previous rounds.
  opponentChoices : The set of RPS Choices made by the opponent in previous rounds.
  roundResults : The set of round results for previous rounds.
*/
const int MAX_ROUNDS = 50;
int roundsPlayed;
RPSChoice arduinoChoices[MAX_ROUNDS];
RPSChoice opponentChoices[MAX_ROUNDS];
RoundResult roundResults[MAX_ROUNDS];

void printTime(const char* str, unsigned long time){
  Serial.print(time);
  Serial.print(" : ");
  Serial.println(str);
}

void printTime(const char* str) { printTime(str, millis()); }

void millisDelay(unsigned long interval){
  unsigned long startingMillis = millis();
  while ( millis() - startingMillis < interval );
}

bool isBitHigh(long n, int pos){
  // Bits are numbered from right to left
  // 3 2 1 0
  // 1 0 0 1
  unsigned long bt = 1 << pos;
  return ( (n & bt) != 0 );
}

void bin(unsigned int n){
  for ( int i=10; i >= 0; i-- ){
    Serial.print( isBitHigh(n, i) ? "1" : "0" );
  }
  Serial.println();
}

void setLEDsToPattern(unsigned int LEDBits){
  /*
    Sets LEDs to the specified bit pattern, uses singe bits
  */
  int leds[] = { LED1, LED2, LED3, LED4, LED5 };
  const char* inf[] = {"LED5", "LED4", "LED3", "LED2", "LED1" };
  int ledCnt = 5;

  for(int bitPos=ledCnt-1; bitPos >= 0; bitPos--){
    if ( isBitHigh(LEDBits, bitPos) )
      digitalWrite(leds[ledCnt-1-bitPos], HIGH);
    else
      digitalWrite(leds[ledCnt-1-bitPos], LOW);
  }

}

void setLEDsToDoubleBitPattern(unsigned int pattern, int blinkingLEDValue){
  /*
    Sets the LEDs to the specified double bit pattern, takes a blinking LED value for LEDs that are set to be blinking
    Uses double bits, see the enum LEDDoubleBitPattern for more information
  */

  unsigned int mask = 0b11;
  unsigned int curLedBits;

  for(int ledPos=0; ledPos < LED_COUNT; ledPos++){
    // Get the bit pair for the current LED by
    // shifting pattern so that bit pair is far right
    // and then anding with mask so only bit pairs are visible
    curLedBits = (pattern >> (ledPos*2)) & mask;

    if ( curLedBits == LED_ON){
      digitalWrite(LEDS[ledPos], HIGH);
    }
    else if (curLedBits == LED_BLINKING)
    {
      digitalWrite(LEDS[ledPos], blinkingLEDValue);
    }
    else
    {
      digitalWrite(LEDS[ledPos], LOW);
    }
  }

}

void setLEDsToDoubleBitPattern(unsigned int pattern) { setLEDsToDoubleBitPattern(pattern, LOW); }

void resetLEDs(){ setLEDsToDoubleBitPattern(LED_OFF); }

unsigned int LEDPatternForGamResult(RoundResult result){
  /*
    Returns the associated LED Pattern that represents a specific round result
  */
  unsigned int pattern;
  switch ( result ){
    case Round_Draw:
      pattern = Draw;
      break;

    case Round_Won:
      pattern = ArduinoWonRound;
      break;

    case Round_Lost:
      pattern = OpponentWonRound;
      break;

    default:
    return;
  }
  return pattern;
}

RPSChoice RBSBtnToChoice (int RPSBtn){
  /*
    Takes one of the RPS buttons and returns the associated enum for that RPS option
  */
  switch ( RPSBtn ){
    case ROCK_BTN:
      return RPS_Rock;

    case PAPER_BTN:
      return RPS_Paper;

    case SCR_BTN:
      return RPS_Scissors;

    default:
      Serial.println("RPS Button had no match!");
      return RPS_None;
  }
}

unsigned int LEDPatternForRPSChoice(RPSChoice choice, bool isOpponent){
  /*
    Returns the associated LED Pattern that represents a specific RPS Choice
    isOpponent is used to determine which player light should be on.
  */
  unsigned int choicePattern;
  unsigned int player = LED_OFF;

  switch(choice){
    case RPS_Rock:
      choicePattern = ShowingRPSRock;
      break;

    case RPS_Paper:
      choicePattern = ShowingRPSPaper;
      break;

    case RPS_Scissors:
      choicePattern = ShowingRPSScissors;
      break;

    default:
      return;
  }
  return player | choicePattern;
}

void blinkLEDs(unsigned int pattern, unsigned long interval, int blinkCount){
  /*
    Takes an LED Double Bit Pattern and blinks leds that have their bit pair set to blinking
  */

  resetLEDs();

  while ( blinkCount > 0 ){
    // set the state of the LEDs
    setLEDsToDoubleBitPattern(pattern, HIGH);
    millisDelay(interval);
    setLEDsToDoubleBitPattern(pattern, LOW);
    millisDelay(interval);

    blinkCount--;
  }

  resetLEDs();
}

int waitForBtnPressAndBlinkLEDs(int btns[], int btnCount, unsigned int ledPattern){
  /*
    Waits for any of the given buttons to be pressed, while blinking the LEDs at the specified interval
    Returns the pin of the button pressed
    ledPattern is an led double bit pattern
  */

  if ( btnCount < 1 ) return -1;

  int pressedBtn = -1;
  unsigned long start;
  unsigned long interval = 500;
  unsigned long cur;
  int curInterval = 0, prevInterval = -1 ;
  int curBlinkingValue = HIGH;

  // LEDS start high
  setLEDsToDoubleBitPattern(ledPattern, curBlinkingValue);
  start = millis();

  while ( pressedBtn == -1 ){

    // check the button for state
    for(int i=0; i < btnCount; i++) {
      if (digitalRead(btns[i]) == BTN_PRESSED_SIGNAL) {
        pressedBtn = btns[i];
        // block till button is released
        while ( digitalRead(pressedBtn) == BTN_PRESSED_SIGNAL );
        break;
      }
    }

    if ( pressedBtn != -1 ) break;

    // set the state of the LEDs
    cur = millis();
    curInterval = (int)((cur - start) / interval);
    if (cur != start && (cur - start) % interval  == 0 && prevInterval != curInterval ){
      // If an interval has passed we have to toggle the LEDs
      curBlinkingValue = !curBlinkingValue;
      setLEDsToDoubleBitPattern(ledPattern, curBlinkingValue);

      // set the new interval
      prevInterval = curInterval;
    }
  }

  resetLEDs();
  return pressedBtn; 
}

int waitForBtnPressAndBlinkLEDs(int btn, unsigned int ledPattern){
  int btnArr[] = { btn };
  return waitForBtnPressAndBlinkLEDs(btnArr, 1, ledPattern);
}


RoundResult determineRoundResults(RPSChoice arduinoChoice, RPSChoice opponentChoice){
  /*
    Returns enum indicating whether arduino won, lost or was a draw.
  */

  // should never have RPS_None here but handle it just in case
  if ( arduinoChoice == RPS_None || opponentChoice == RPS_None ){
    return Round_None;
  }

  /*
    We can use simple math to solve the problem
    Since rock < paper < scissors < rock ...,
    Is cyclic, we can use modulo, hence
    rock = 0
    paper = 1
    scissors = 2

    For a given choice at i, choice at (i+1 % 3) always beats choice at i
  */

  if ( opponentChoice == (arduinoChoice+1) % 3 ){
    // opponent selected choice that beats Arduino
    return Round_Lost;
  }
  else if ( opponentChoice == arduinoChoice ){
    return Round_Draw;
  } else {
    return Round_Won;
  }
}


void saveRound(RPSChoice arduinoChoice, RPSChoice opponentChoice, RoundResult roundResult){
  /*
    Saves the results of the round in the arrays and increments our roundCount
    arduinoChoice is saved in arduinoChoices array
    opponentChoice is saved in opponentChoices array
    roundResult is saved in roundResults array
  */

  arduinoChoices[roundsPlayed] = arduinoChoice;
  opponentChoices[roundsPlayed] = opponentChoice;
  roundResults[roundsPlayed] = roundResult;
  roundsPlayed++;
}

String RoundResToWinner(RoundResult gs){
  /*
    Returns the string of the round winner based on the round result
  */
  switch(gs){
    case Round_Draw:
      return "Draw";

    case Round_Won:
      return "Arduino";

    case Round_Lost:
      return "Opponent";

    case Round_None:
      return "None";
  }
}

String RPSChoiceToStr(RPSChoice choice) {
  /*
    Returns the string version of an RPS choice
  */
  switch(choice){
    case RPS_Rock:
      return "Rock";

    case RPS_Paper:
      return "Paper";

    case RPS_Scissors:
      return "Scissors";

    case RPS_None:
      return "None";
  }
}

RPSChoice randRPS(){
  /*
    Returns a random rps choice
  */
  return (RPSChoice)random(RPS_Rock, RPS_Scissors+1);
}

RPSChoice winningChoiceAgainst(RPSChoice choice){
  /*
    Returns the RPS choice that wins against choice.
    e.g. winningChoiceAgainst(RPS_Rock) = RPS_Paper
  */
  return (RPSChoice)((choice+1) % 3);
}

RPSChoice losingChoiceAgainst(RPSChoice choice){
  /*
    Returns the RPS choice that loses against choice.
    e.g. losingChoiceAgainst(RPS_Rock) = RPS_Scissors
  */
  return (RPSChoice)((choice+2) % 3);
}

//******************************************************************
// TODO Modify this function
//******************************************************************

//Our Global Variables
int trans[3][3] = {{0}}; //transition counts
int epsPercent = 15; //exploration probability (5%-25%)
int noGain = 0; //rounds since last win

RPSChoice makeRPSChoice() { 
  if (roundsPlayed == 0) {
    return randRPS();
  } //First round return random choice

  //update the table once atleast 2 moves are played
  if (roundsPlayed > 1) {
    int p = opponentChoices[roundsPlayed-2];
    int q = opponentChoices[roundsPlayed-1];
    if (p >= 0 && q >= 0)
      trans[p][q]++;
  }

  //pulling info from last round
  int oppPrev = opponentChoices[roundsPlayed-1];
  if (oppPrev < 0) //negative index guard just in case because RPS_None is -1
    return randRPS();
  RoundResult res = roundResults[roundsPlayed-1];

  //predict next move
  int *row = trans[oppPrev];
  RPSChoice pred;
  if (row[0] >= row[1] && row[0] >= row[2])
    pred = RPS_Rock;
  else if (row[1] >= row[2])
    pred = RPS_Paper;
  else
    pred = RPS_Scissors;

  RPSChoice choice = winningChoiceAgainst(pred); //use prediction

  //randomness
  if (random(100) < epsPercent)
    choice = randRPS();

  //adapt & track cold streak
  if (res == Round_Won) {
    noGain = 0;
    if (epsPercent > 5)
      epsPercent -=2;
  }
  else {
    noGain++;
    if (epsPercent < 25)
      epsPercent +=2;
  }

  //auto-reset if stuck
  if (noGain >= 6) {
    memset(trans, 0, sizeof(trans));
    epsPercent = 15;
    noGain = 0;
  }
  
  return choice;
}
//******************************************************************
//******************************************************************


void setup() {
  // Set the frequency of the serial output, will be the frequency required in the serial monitor
  Serial.begin(9600);

  int outputPins[]= { LED1, LED2, LED3, LED4, LED5 };
  int inputPins[] = { BTN1, BTN2, BTN3, BTN4 };

  /*
    Set the mode for output pins to output
  */
  for ( int pin : outputPins ){
    pinMode(pin, OUTPUT);
  }

  /*
    Set the mode for input pins to input
  */
  for ( int pin : inputPins ){
    pinMode(pin, INPUT);
  }

  // Initialize our round global variables
  roundsPlayed = 0;
  for (int i = 0; i < MAX_ROUNDS; i++){
    arduinoChoices[i] = RPS_None;
    opponentChoices[i] = RPS_None;
    roundResults[i] = Round_None;
  }

  // Initialize seed for random,
  randomSeed(analogRead(0));
  srand(analogRead(0)); 
}

void loop() {

  Serial.println("=======================>");

  if ( roundsPlayed >= MAX_ROUNDS ){
      resetLEDs();
      Serial.println("Out of Rounds");
      return;
  }

  // Wait for the turn button to be pressed
  Serial.println("Waiting for round start...");
  waitForBtnPressAndBlinkLEDs(TURN_BTN, WaitingForRound);
  Serial.print("Started Round: ");
  Serial.println(roundsPlayed+1);

  RPSChoice opponentChoice, arduinoChoice;
  RoundResult result;

  // Arduino makes its choice first
  arduinoChoice = makeRPSChoice();

  // show arduino choice
  Serial.println("Arduino selected: " + RPSChoiceToStr(arduinoChoice));

  // Now listen to get the choice the opponent made
  // While listening  for opponent choice, we show the arduino choice 
  Serial.println("Waiting for opponent choice...");
  opponentChoice = RBSBtnToChoice(waitForBtnPressAndBlinkLEDs(RPS_BTNS, RPS_BTN_COUNT, LEDPatternForRPSChoice(arduinoChoice, false)));

  Serial.println("Opponent selected: " + RPSChoiceToStr(opponentChoice));

  // determine the result
  result = determineRoundResults(arduinoChoice, opponentChoice);

  // save the result
  Serial.println("Saving round information.");
  saveRound(arduinoChoice, opponentChoice, result);
  
    // Show round results on LEDs while listening for end round
  Serial.println("Round Winner: " + RoundResToWinner(result));
  Serial.println("Press turn button to end round...");
  waitForBtnPressAndBlinkLEDs(TURN_BTN, LEDPatternForGamResult(result));
  
  Serial.println("=======================>");
}
