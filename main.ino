// This program turns five buttons and four LEDs into a toy that allows you to play Simon Says, Whack-A-Mole, or a piano.

const int NUM_OF_BUTTONS = 4; // constant to store the number of buttons
const byte BUTTONS[NUM_OF_BUTTONS] = {9, 10, 11, 12}; // the pushbuttons are connected to pins 2, 5, 8, and 11
const byte BUZZER = 6; // the buzzer is connected to pin 10
const byte PAUSE_BUTTON = 8; // the pause button is connected to pin 4
const byte LEDS[NUM_OF_BUTTONS] = {2, 3, 4, 5}; // the LEDs are connected to pins 2, 3, 4, and 5

const int BUZZER_FREQUENCIES[NUM_OF_BUTTONS] = {220, 262, 330, 440}; // the frequencies that each button plays (A, C, E, A)
const int WRONG_FREQUENCY = 100; // the frequency that plays when the user gets something wrong

const int OUTPUT_LENGTH = 500; // constant that stores the amount of time that a light stays on when displaying a sequence
const int OUTPUT_DELAY = 100; // constant that stores the amount of time between lights when displaying a sequence
const int WRONG_LENGTH = 1500; // constant that stores the length of the sound played when the user gets it wrong
const int FEEDBACK_DELAY = 2000; // constant that stores the delay before showing the correct sequence after the user gets it wrong
const int LEVEL_DELAY = 2000; // constant that stores the delay between "levels"
const int START_DELAY = 3000; // constant that stores the delay before starting the game

const int MAX_DELAY = 5000; // constant that stores the maximum amount of time the user has to choose a button
const int CHECKING_DELAY = 50; // constant that stores the amount of time between checking for user input

const int MAX_SCORE = 128; // a maximum score is necessary for the memory games to prevent the Arduino from running out of memory

const int MAX_MOLES = 3; // the maximum number of moles that can appear simultaneously

const int MAX_MOLE_TIME = 2000; // the maximum time a mole can stay alive
const int MIN_MOLE_TIME = 100; // the minimum time a mole can stay alive
const int MOLE_TIME_DECREMENT = 100; // the amount the mole time is decremented each time
const int MOLE_TIME_DTIME = 3000; // the amount of time between decrements of the mole time

const int MAX_MOLE_DELAY = 2000; // the maximum time between moles being generated
const int MIN_MOLE_DELAY = 300; // the minimum time between moles being generated
const int MOLE_DELAY_DECREMENT = 100; // the amount the mole delay is decremented each time
const int MOLE_DELAY_DTIME = 3000; // the amount of time between decrements of the mole delay


enum Game {easy = 0, normal = 1, mole = 2, piano = 3}; // enum to store the different games the user could choose
enum Game gameChoice; // variable that stores the game that the user chooses

struct Mole { // struct that stores information about a mole
  unsigned long born; // variable that stores when the mole was created
  int lifetime; // variable that stores how long the mole should stay alive for
};

void endGame(int score) { // function that ends the game
  Serial.print("You ended with a score of "); // tells the user that the game ended
  Serial.print(score); // tells the user what their score was
  Serial.println("!"); // exclamation point + line break
  while (1) {} // infinite loop; ends the game
}

byte readButtonInput(bool haveMaxDelay, int score) { // function that reads a button input
  unsigned long startTime = millis(); // constant that stores the current time
  
  while (1) { // while loop that runs until the user presses a button
    
    if (haveMaxDelay && ((millis() - startTime) > MAX_DELAY)) { // checks if too much time has been waited
      Serial.println("Too much time has passed; game ended."); // tells the user that too much time has passed and the game has ended
      endGame(score); // ends the game
    }
    
    checkForPause(score, &startTime); // checks for a pause
    
    for (int i = 0; i < NUM_OF_BUTTONS; i++) { // for loop that loops through each button
      if (!digitalRead(BUTTONS[i])) { // checks if the button is pressed
        tone(BUZZER, BUZZER_FREQUENCIES[i]); // plays the corresponding frequency on the buzzer
        digitalWrite(LEDS[i], HIGH); // turns on the corresponding LED
        while (!digitalRead(BUTTONS[i])) {} // waits for the button to be released
        noTone(BUZZER); // stops the buzzer
        digitalWrite(LEDS[i], LOW); // turns off the corresponding LED
        return i; // returns the index of the button that the user pressed
      }
    }
  }
}

void displaySequence(byte *sequence, int sequenceLength) { // function that displays a sequence of LEDs
  
  for (int i = 0; i < sequenceLength; i++) { // for loop that loops through the sequence
    const byte ledIndex = sequence[i]; // constant that stores the current element of the sequence
    
    tone(BUZZER, BUZZER_FREQUENCIES[ledIndex]); // plays the corresponding frequency on the buzzer
    digitalWrite(LEDS[ledIndex], HIGH); // turns on the corresponding LED
    delay(OUTPUT_LENGTH); // waits an amount of time
    noTone(BUZZER); // stops the buzzer
    digitalWrite(LEDS[ledIndex], LOW); // turns off the corresponding LED
    delay(OUTPUT_DELAY); // waits an amount of time
  }
  
}

void checkForPause(int score, unsigned long *startTime) { // function that checks if the pause button is pressed, and if it is, pauses whack-a-mole
  if (!digitalRead(PAUSE_BUTTON)) { // checks if the button is pressed
    unsigned long startOfPause = millis(); // variable that stores when the pause started
    
    while (!digitalRead(PAUSE_BUTTON)) {} // waits for the button to be released
    Serial.print("Paused the game. Your current score is "); // tells the user that the game was paused
    Serial.print(score); // tells the user what their current score is
    Serial.println("."); // period + line break
    while (digitalRead(PAUSE_BUTTON)) {} // waits for the button to be pressed
    while (!digitalRead(PAUSE_BUTTON)) {} // waits for the button to be released
    Serial.println("Continuing the game."); // tells the user that the game is continuing
    
    *startTime += millis() - startOfPause; // offsets the pause time so that it does not affect the game
  }
}

int chooseMole(int numOfMoles, bool *moleIsAlive) { // function that chooses a mole based on which moles are alive
  int newMole = random(NUM_OF_BUTTONS - numOfMoles); // chooses a random number that corresponds to one of the dead moles (but we don't know which one that is yet)
  
  for (int i = 0; i <= newMole; i++) { // loops until i is greater than newMole
    if (moleIsAlive[i]) { // checks if this mole is alive
      newMole++; // if so, increment newMole to move on to the next mole
    }
  }
  
  return newMole; // returns the new mole index
}

void easyMemory() { // function that plays the easy memory game

  byte sequence[MAX_SCORE]; // array to store the sequence
  
  for (int length = 1; length <= MAX_SCORE; length++) { // for loop that loops until the maximum score is achieved
    sequence[length - 1] = random(NUM_OF_BUTTONS); // chooses a random button and adds it to the end of the array
    
    displaySequence(sequence, length); // displays the current sequence
    
    for (int i = 0; i < length; i++) { // loops through the sequence
      byte buttonInput = readButtonInput(true, length - 1); // gets a button input from the user
      if (buttonInput != sequence[i]) { // if the button is not the corresponding part of the sequence...
        tone(BUZZER, WRONG_FREQUENCY, WRONG_LENGTH); // plays the "incorrect sound"
        Serial.println("Incorrect. Here was the correct sequence:"); // tells the user they were incorrect
        delay(FEEDBACK_DELAY); // waits an amount of time
        displaySequence(sequence, length); // displays the sequence again
        endGame(length - 1); // ends the game
      }
    }
    
    Serial.println("Correct!"); // if the code reaches this point, the user was correct; this tells the user they were correct
    
    delay(LEVEL_DELAY); // waits an amount of time between levels
  }
  
  
  Serial.print("You achieved the maximum score of "); // tells the user they achieved the maximum score
  Serial.println(MAX_SCORE); // tells the user what the maximum score is
}

void normalMemory() { // function that plays the normal memory game

  for (int length = 1; length <= MAX_SCORE; length++) { // for loop that loops until the maximum score is achieved
  
    byte sequence[length]; // creates an array of the current length
    
    for (int i = 0; i < length; i++) { // loops through the sequence
      sequence[i] = random(NUM_OF_BUTTONS); // chooses a random button and stores it in the array
    }
    
    displaySequence(sequence, length); // displays the current sequence
    
    for (int i = 0; i < length; i++) { // loops through the sequence
      byte buttonInput = readButtonInput(true, length - 1); // gets a button input from the user
      if (buttonInput != sequence[i]) { // if the button is not the corresponding part of the sequence...
        tone(BUZZER, WRONG_FREQUENCY, WRONG_LENGTH); // plays the "incorrect sound"
        Serial.println("Incorrect. Here was the correct sequence:"); // tells the user they were incorrect
        delay(FEEDBACK_DELAY); // waits an amount of time
        displaySequence(sequence, length); // displays the sequence again
        endGame(length - 1); // ends the game
      }
    }
    
    Serial.println("Correct!"); // if the code reaches this point, the user was correct; this tells the user they were correct
    
    delay(LEVEL_DELAY); // waits an amount of time between levels
  }
  
  Serial.print("You achieved the maximum score of "); // tells the user they achieved the maximum score
  Serial.println(MAX_SCORE); // tells the user what the maximum score is
}

void whackAMole() { // function that plays whack-a-mole

  unsigned long startTime = millis(); // variable that stores the time when the game started
  
  int moleTime = MAX_MOLE_TIME; // variable that stores the time a new mole lives for
  struct Mole remainingMoleTime[NUM_OF_BUTTONS]; // array that stores information about every mole
  unsigned long lastMoleTimeDecrease = startTime; // variable that stores when the last decrease in mole lifetime happened
  
  int moleDelay = MAX_MOLE_DELAY; // variable that stores the amount of time between new moles
  unsigned long lastMole = startTime; // variable that stores when the last mole was created
  unsigned long lastMoleDelayDecrease = startTime; // variable that stores when the last decrease in mole delay happened
  
  int numOfMoles = 0; // variable that stores the number of moles
  
  bool moleIsAlive[NUM_OF_BUTTONS] = {false}; // array that stores which moles are alive
  bool isPressed[NUM_OF_BUTTONS] = {false}; // array that stores which buttons are pressed
  
  while (1) { // while loop that loops until the user presses the wrong button or does not whack a mole in time
    
    checkForPause((millis() - startTime) / 1000, &startTime); // checks for a pause
    
    const unsigned long currentTime = millis(); // constant that stores the current time
    
    if (((currentTime - lastMole) > moleDelay) && (numOfMoles < MAX_MOLES)) { /* checks if it is time to create a mole AND if the maximum number of moles has not been
                                                                                 met */
      int newMole = chooseMole(numOfMoles, moleIsAlive); // chooses a new mole and stores it
      
      remainingMoleTime[newMole] = (struct Mole) {currentTime, moleTime}; // stores the new mole's data
      moleIsAlive[newMole] = true; // stores that this mole is noew alive
      lastMole = currentTime; // stores that the last mole was created now
      numOfMoles++; // increments the number of moles
      
      digitalWrite(LEDS[newMole], HIGH); // turns on the corresponding LED
    }
    
    for (int i = 0; i < NUM_OF_BUTTONS; i++) { // for loop that loops through the buttons/LEDs
      if (((currentTime - remainingMoleTime[i].born) > remainingMoleTime[i].lifetime) && moleIsAlive[i]) { // checks if a mole has been alive for too long
        noTone(BUZZER); // stops the buzzer
        tone(BUZZER, WRONG_FREQUENCY, WRONG_LENGTH); // plays the "incorrect sound"
        endGame((currentTime - startTime) / 1000); // ends the game
      }
      
      if (!digitalRead(BUTTONS[i])) { // checks if the button is pressed
        if (!isPressed[i]) { // checks if the array says the button was not pressed
          if (moleIsAlive[i]) { // checks if the mole is alive
            digitalWrite(LEDS[i], LOW); // turns off the corresponding LED
            moleIsAlive[i] = false; // kills the mole
            numOfMoles--; // decrements the number of moles
          } else { // if the mole is dead, the game should end because the user pressed the wrong button
            noTone(BUZZER); // stops the buzzer
            tone(BUZZER, WRONG_FREQUENCY, WRONG_LENGTH); // plays the "incorrect sound"
            endGame((currentTime - startTime) / 1000); // ends the game
          }
          
          tone(BUZZER, BUZZER_FREQUENCIES[i]); // plays the corresponding frequency on the buzzer
          isPressed[i] = true; // updates the array to say the button is pressed
        }
        
      } else if (isPressed[i]) { // if the button is not pressed AND the array says it was pressed
        noTone(BUZZER); // stops the buzzer
        isPressed[i] = false; // updates the array to say the button is not pressed
      }
    }
    
    if (((currentTime - lastMoleTimeDecrease) >= MOLE_TIME_DTIME) && ((moleTime - MOLE_TIME_DECREMENT) > MIN_MOLE_TIME)) { /* checks if it is time to decrement the mole
                                                                                                                              lifetime AND the minimum mole time has not
                                                                                                                              been reached yet */
      moleTime -= MOLE_TIME_DECREMENT; // decrements the mole time
      lastMoleTimeDecrease = currentTime; // stores that the mole time was decreased now
    }
    
    if (((currentTime - lastMoleDelayDecrease) >= MOLE_DELAY_DTIME) && ((moleDelay - MOLE_DELAY_DECREMENT) > MIN_MOLE_DELAY)) { /* checks if it is time to decrement the
                                                                                                                                   mole delay AND the minimum mole delay
                                                                                                                                   has not been reached yet */
      moleDelay -= MOLE_DELAY_DECREMENT; // decrements the mole delay
      lastMoleDelayDecrease = currentTime; // stores that the mole delay was decreased now
    }
  } 
}

void pianoMode() { // function that plays the piano

  bool isBeingPlayed[NUM_OF_BUTTONS] = {false}; // array that stores which buttons are pressed
  while (1) { // while loop that runs forever
    for (int i = 0; i < NUM_OF_BUTTONS; i++) { // for loop that loops through the buttons
      if (!digitalRead(BUTTONS[i]) && !isBeingPlayed[i]) { // checks if the button is pressed and the array says that it was not pressed
        tone(BUZZER, BUZZER_FREQUENCIES[i]); // plays the corresponding frequency on the buzzer
        digitalWrite(LEDS[i], HIGH); // turns on the corresponding LED
        isBeingPlayed[i] = true; // stores that the button is pressed
      } else if (digitalRead(BUTTONS[i]) && isBeingPlayed[i]) { // checks if the button is not pressed and the array says that it was pressed
        noTone(BUZZER); // stops the buzzer
        digitalWrite(LEDS[i], LOW); // turns off the corresponding LED
        isBeingPlayed[i] = false; // stores that the button is not pressed
      }
    }
  }
  
}

void setup() { // setup function
  Serial.begin(9600); // sets the bit rate for the serial monitor
  
  pinMode(PAUSE_BUTTON, INPUT_PULLUP); // sets the pause button pin to use the internal pull-up resistor for input
  pinMode(BUZZER, OUTPUT); // sets the buzzer to output
  
  for (int i = 0; i < NUM_OF_BUTTONS; i++) { // for loop that loops through the buttons
    pinMode(BUTTONS[i], INPUT_PULLUP); // sets the button to use the internal pull-up resistor for input
    pinMode(LEDS[i], OUTPUT); // sets the LED to output
  }
  
  randomSeed(analogRead(0)); // sets a random seed, reading an unconnected pin for a random value
  
  Serial.println("Choose a game:"); // prints a prompt asking the user to select a difficulty
  Serial.println("red - easy memory"); // pressing the red button starts an easy memory game
  Serial.println("yellow - normal memory"); // pressing the yellow button starts a normal memory game
  Serial.println("green - whack-a-mole"); // pressing the green button starts a whack-a-mole game
  Serial.println("blue - piano"); // pressing the blue button starts the piano
  
  while (1) { // a while loop that runs until the user enters a valid input
    
    byte difficultyInput = readButtonInput(false, 0); // gets the user input
    
    if (difficultyInput == 0) { // if the user pressed the red button...
      gameChoice = easy; // sets the game to the easy memory game
      Serial.println("Easy memory selected."); // tells the user that easy memory was selected
      break; // breaks from the loop
    } else if (difficultyInput == 1) { // if the user pressed the yellow button...
      gameChoice = normal; // sets the game to the normal memory game
      Serial.println("Normal memory selected."); // tells the user that normal memory was selected
      break; // breaks from the loop
    } else if (difficultyInput == 2) { // if the user pressed the green button...
      gameChoice = mole; // sets the game to whack-a-mole
      Serial.println("Whack-a-mole selected."); // tells the user that whack-a-mole was selected
      break; // breaks from the loop
    } else if (difficultyInput == 3) { // if the user pressed the blue button...
      gameChoice = piano; // sets the game to the piano
      Serial.println("Piano selected."); // tells the user that the piano was selected
      break; // breaks from the loop
    }
  }
  
  delay(START_DELAY); // waits some time before starting the actual game
}

void loop() { // loop function that plays the chosen game
  Serial.println("Starting the game."); // tells the user that the game is starting
  
  if (gameChoice == easy) { // if the user chose the easy memory game...
    easyMemory(); // starts the easy memory game
  } else if (gameChoice == normal) { // if the user chose the normal memory game...
    normalMemory(); // starts the normal memory game
  } else if (gameChoice == mole) { // if the user chose whack-a-mole
    whackAMole(); // starts whack-a-mole
  } else if (gameChoice == piano) { // if the user chose the piano
    pianoMode(); // starts the piano
  }
}
