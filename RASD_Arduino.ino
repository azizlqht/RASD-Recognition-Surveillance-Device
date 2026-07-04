#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9
#define GREEN_LED 3
#define IR_PIN 2
#define CAMERA_BUTTON 4  // Pin connected to the camera trigger button

MFRC522 mfrc522(SS_PIN, RST_PIN);

// System states
enum SystemState {
  WAITING_FOR_CARD,      // Waiting for an authorized card
  CARD_AUTHORIZED,       // Card authorized - waiting for IR
  LIGHT_ON,              // Light is on (RFID + IR both satisfied)
  IR_DETECTED_WAIT_RFID, // IR detected - waiting for RFID
  IR_ONLY_MODE,          // IR-only state
  CAMERA_ACTIVE          // Photo capture state (camera active)
};

SystemState currentState = WAITING_FOR_CARD;
String lastCardUID = "";
bool lastIRState = HIGH;
bool irTriggered = false;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;
unsigned long irDetectionTime = 0;
const unsigned long rfidWaitTime = 3000; // 3 second wait for RFID
unsigned long cameraStartTime = 0;
const unsigned long cameraDuration = 6000; // 6 seconds for photo capture

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();

  pinMode(GREEN_LED, OUTPUT);
  pinMode(IR_PIN, INPUT_PULLUP);
  pinMode(CAMERA_BUTTON, OUTPUT);
  
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(CAMERA_BUTTON, LOW);
  
  Serial.println("System Started");
}

void loop() {
  bool irDetected = (digitalRead(IR_PIN) == LOW);
  
  // Debounce for the IR sensor
  if (irDetected != lastIRState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (irDetected != irTriggered) {
      irTriggered = irDetected;
      
      // Detect IR state change
      if (irTriggered) {
        // IR detected
        handleIRDetected();
      } else {
        // IR removed
        handleIRRemoved();
      }
    }
  }
  
  lastIRState = irDetected;
  
  // Check RFID in all states except IR_ONLY_MODE and CAMERA_ACTIVE
  if (currentState != IR_ONLY_MODE && currentState != CAMERA_ACTIVE) {
    checkRFID();
  }
  
  // Check active camera state
  if (currentState == CAMERA_ACTIVE) {
    checkCameraDuration();
  }
  
  switch (currentState) {
    case WAITING_FOR_CARD:
      digitalWrite(GREEN_LED, LOW);
      break;
      
    case CARD_AUTHORIZED:
      if (irTriggered) {
        currentState = LIGHT_ON;
        digitalWrite(GREEN_LED, HIGH);
        Serial.println("Light ON - Both conditions met");
      }
      
      // Return to reading a new card if the reader withdrew
      if (!irTriggered) {
        currentState = WAITING_FOR_CARD;
      }
      break;
      
    case LIGHT_ON:
      // Stay in LIGHT_ON state until IR state changes
      if (!irTriggered) {
        currentState = WAITING_FOR_CARD;
        digitalWrite(GREEN_LED, LOW);
        Serial.println("Light OFF - IR changed, waiting for new card");
      }
      break;
      
    case IR_DETECTED_WAIT_RFID:
      // Check if the wait time has expired
      if (millis() - irDetectionTime >= rfidWaitTime) {
        // Wait time expired and no RFID came
        Serial.println("RFID wait time expired - Taking photo");
        currentState = CAMERA_ACTIVE;
        triggerCamera();
      }
      break;
      
    case IR_ONLY_MODE:
      // In IR-only state, do nothing but wait for IR removal
      if (!irTriggered) {
        currentState = WAITING_FOR_CARD;
        Serial.println("Exiting IR only mode");
      }
      break;
      
    case CAMERA_ACTIVE:
      // Camera active - handled in checkCameraDuration()
      break;
  }
  
  delay(50); // Small delay to avoid rapid repeated reads
}

void handleIRDetected() {
  Serial.println("IR Detected");
  
  // If waiting for a card (no RFID yet) and IR is detected
  if (currentState == WAITING_FOR_CARD) {
    currentState = IR_DETECTED_WAIT_RFID;
    irDetectionTime = millis();
    Serial.println("Waiting for RFID for 3 seconds...");
  }
}

void handleIRRemoved() {
  Serial.println("IR Removed");
  
  // If waiting for RFID and IR was removed
  if (currentState == IR_DETECTED_WAIT_RFID) {
    currentState = WAITING_FOR_CARD;
    Serial.println("IR removed before RFID wait time expired");
  }
  // If in IR-only state
  else if (currentState == IR_ONLY_MODE) {
    currentState = WAITING_FOR_CARD;
    Serial.println("Exiting IR only mode");
  }
}

void checkRFID() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String cardUID = readCardUID();
    
    Serial.print("RFID Detected: ");
    Serial.println(cardUID);
    
    // If waiting for RFID after IR
    if (currentState == IR_DETECTED_WAIT_RFID) {
      if (isAuthorizedCard(cardUID)) {
        // RFID authorized - turn on light
        lastCardUID = cardUID;
        currentState = LIGHT_ON;
        digitalWrite(GREEN_LED, HIGH);
        Serial.println("Authorized RFID after IR - Light ON");
      } else {
        // RFID unauthorized - return to waiting
        Serial.println("Unauthorized RFID - Returning to wait state");
        currentState = WAITING_FOR_CARD;
      }
    }
    // If in normal waiting state
    else if (currentState == WAITING_FOR_CARD) {
      if (isAuthorizedCard(cardUID)) {
        lastCardUID = cardUID;
        currentState = CARD_AUTHORIZED;
        Serial.println("Card Authorized - Waiting for IR");
      } else {
        Serial.println("Unauthorized Card");
      }
    }
  }
}

void triggerCamera() {
  // Start photo capture for 6 seconds
  Serial.println("Starting photo capture for 6 seconds...");
  
  // Activate the camera pin
  digitalWrite(CAMERA_BUTTON, HIGH);
  cameraStartTime = millis();
  
  Serial.println("Camera button pressed for 6 seconds");
}

void checkCameraDuration() {
  // Check camera button press duration
  if (millis() - cameraStartTime >= cameraDuration) {
    // 6 seconds ended - turn off camera
    digitalWrite(CAMERA_BUTTON, LOW);
    currentState = IR_ONLY_MODE;
    Serial.println("Photo capture completed (6 seconds) - Camera button released");
  }
}

String readCardUID() {
  String content = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  return content.substring(1);
}

bool isAuthorizedCard(String cardUID) {
  // Authorized card
  if (cardUID == "E7 2A 99 60") {
    return true;
  }
  
  return false;
}
