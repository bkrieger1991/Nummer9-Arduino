#include <Arduino.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include <pRNG.h>

// Pin Definitions
// DFPlayer Busy Pin: signals whether a track is played or not
#define BUSY_PIN 8
// DFPlayer TX (transmit) pin, used for serial communication
#define PLAYER_TX_PIN 9
// DFPlayer RX (receive) pin, used for serial communication
#define PLAYER_RX_PIN 10
// PIR Motion sensor signal pin
#define SENSOR_PIN 11
// Debug pin (LOW = no debugging)
#define DEBUG_PIN 12
// Status LED pin
#define LED_PIN 13

// === Settings ===
// DFPlayer settings
#define PLAYER_VOLUME 5
#define PLAYER_TIMEOUT 2000
// Time between the end of one track and the start of another
#define TRACK_DELAY 5000
// Delay at the end of each loop
#define LOOP_DELAY 800
// Delay after starting a track with next()
#define PLAYER_PLAY_DELAY 500
// Setup delay after initializing
#define INIT_DELAY 2000

// millis() since playback was finished
unsigned long lastTrackFinished = 0;
bool isPlaying = false;
bool debug = false;

SoftwareSerial playerSerial(PLAYER_TX_PIN, PLAYER_RX_PIN);
DFRobotDFPlayerMini dfPlayer; 

void print(String m);
void println(String m);

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(DEBUG_PIN, INPUT);
  pinMode(SENSOR_PIN, INPUT);
  pinMode(BUSY_PIN, INPUT_PULLUP);

  // Debug mode is disabled by setting pin to LOW
  debug = digitalRead(DEBUG_PIN) != LOW;
  if(debug) {
    Serial.begin(9600);
  }
  playerSerial.begin(9600);

  print("Initializing...");
  if (!dfPlayer.begin(playerSerial)) {
    println("mp3 player serial error...");
    // Fehler beim Initialisieren des DFPlayers
    while (true) {
      digitalWrite(LED_BUILTIN, HIGH);  // LED bleibt an, um Fehler anzuzeigen
      delay(200);
      digitalWrite(LED_BUILTIN, LOW);
      delay(200);
    }
  }

  print("player connected...");
  
  dfPlayer.volume(PLAYER_VOLUME);
  dfPlayer.outputDevice(DFPLAYER_DEVICE_SD);
  dfPlayer.EQ(DFPLAYER_EQ_JAZZ);
  dfPlayer.setTimeOut(PLAYER_TIMEOUT);
  
  delay(INIT_DELAY);
  
  lastTrackFinished = millis();
  println("Done.");
}

void loop() {
  int sensorValue = digitalRead(SENSOR_PIN);

  // Read busy state of DFPlayer (HIGH = not busy)
  // Turn off status LED and save last playback time when playback was active
  if(digitalRead(BUSY_PIN) == HIGH) {
    digitalWrite(LED_PIN, LOW);
    if(isPlaying) {
      lastTrackFinished = millis();
    }
    isPlaying = false;
  // Turn on status LED since device is busy
  } else {
    digitalWrite(LED_PIN, HIGH);
  }

  // Calculate time delta since last track finished
  int msSinceLastTrackFinished = millis() - lastTrackFinished;

  print("Checking for motion...");
  if (sensorValue == HIGH) {
    print("Motion detected...");
    bool isInTrackDelay = msSinceLastTrackFinished <= TRACK_DELAY;
    if(!isPlaying && !isInTrackDelay) {
      print("Play track...");
      isPlaying = true;
      dfPlayer.next();
      delay(PLAYER_PLAY_DELAY);
    } else if(isPlaying) {
      print("Still playing...");
    } else if(isInTrackDelay) {
      print("Remaining delay: " + String(TRACK_DELAY - msSinceLastTrackFinished) + "ms...");
    }
  } else {
    print("No motion detected...");  
  }

  println("Loop done.");
  delay(LOOP_DELAY);
}

void print(String m) {
  if(debug) {
    Serial.print(m);
  }
}

void println(String m) {
  if(debug) {
    Serial.println(m);
  }
}