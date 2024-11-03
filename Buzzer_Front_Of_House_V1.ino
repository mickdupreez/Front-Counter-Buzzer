//DEVICE 1 Buzzer Button On Front Counter

#include <LiquidCrystal_I2C.h>  // Include library for controlling the LCD display over I2C
#include <esp_now.h>            // Include ESP-NOW library for peer-to-peer communication
#include <WiFi.h>               // Include WiFi library, required for ESP-NOW functionality

// Define GPIO pin for the button that users press to request assistance
#define BUTTON_PIN 3

// Define timing delays and intervals (in milliseconds) for debounce, display, etc.
#define DEBOUNCE_DELAY 500               // Delay to prevent accidental double-presses
#define MIN_TRIGGER_INTERVAL 1000        // Minimum time interval between button presses
#define DISPLAY_DELAY 3000               // Duration to display each message

// Define I2C address and pins for the LCD display
#define LCD_ADDR 0x27                    // I2C address for the LCD
#define SDA 7                            // I2C data pin
#define SCL 8                            // I2C clock pin

// Create an instance of the LCD display with address and size (16x2 characters)
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

// Define the MAC address of the receiver device (Device 2) to send the ESP-NOW signal to
uint8_t receiverMAC[] = {0xC8, 0x2E, 0x18, 0x6B, 0xE3, 0x48};

// Variables for managing button press timing and display states
unsigned long lastButtonPressTime = 0;     // Time of the last button press, used for debounce
unsigned long lastMessageChangeTime = 0;   // Time when the last message was displayed
bool buttonPressed = false;                // Tracks whether the button has been pressed
int messageIndex = 0;                      // Tracks which message to display

// Function to display a message on the LCD screen
// line1 and line2 are the two lines of text to show
void displayMessage(const char* line1, const char* line2) {
  lcd.clear();                // Clear any existing text on the display
  lcd.setCursor(0, 0);        // Set the cursor to the beginning of the first line
  lcd.print(line1);           // Print the first line of the message
  lcd.setCursor(0, 1);        // Move the cursor to the beginning of the second line
  lcd.print(line2);           // Print the second line of the message
}

// Setup function that runs once when the device is powered on or reset
void setup() {
  // Initialize serial communication for debugging purposes (optional)
  Serial.begin(115200);

  // Set up the button pin as an input
  pinMode(BUTTON_PIN, INPUT);

  // Initialize I2C communication with specified SDA and SCL pins
  Wire.begin(SDA, SCL);

  // Initialize the LCD display with 16 columns and 2 rows, and enable the backlight
  lcd.begin(16, 2);
  lcd.init();
  lcd.backlight();

  // Set up ESP-NOW communication and ensure WiFi is in station mode
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW"); // Log an error if ESP-NOW fails to initialize
    return;
  }

  // Register the receiver device as a peer to allow ESP-NOW communication
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, receiverMAC, 6);  // Set the receiver's MAC address
  peerInfo.channel = 0;                        // Set the WiFi channel to 0
  peerInfo.encrypt = false;                    // Disable encryption for this example
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");      // Log an error if adding the peer fails
    return;
  }
}

// Function to send a message via ESP-NOW to the receiver
// This sends the value '1' as a signal to Device 2
void sendESPNowMessage() {
  uint8_t data = 1;  // The data to be sent, here we're using a single byte with value '1'
  esp_now_send(receiverMAC, &data, sizeof(data));  // Send data to the device with receiverMAC
}

// Loop function that continuously checks for button presses and displays messages
void loop() {
  // Read the current button state to check if it's pressed
  bool currentButtonState = digitalRead(BUTTON_PIN);
  
  // Get the current time in milliseconds for timing-related checks
  unsigned long currentTime = millis();

  // Check if the button is pressed (button state HIGH) and meets debounce criteria
  if (currentButtonState == HIGH && 
      (currentTime - lastButtonPressTime > DEBOUNCE_DELAY) &&  // Debounce: avoid multiple detections for single press
      (currentTime - lastButtonPressTime > MIN_TRIGGER_INTERVAL)) {  // Ensure minimum interval between presses

    buttonPressed = true;                            // Indicate that the button was pressed
    lastButtonPressTime = currentTime;               // Update last press time for future debounce
    messageIndex = 0;                                // Reset message index for new sequence

    sendESPNowMessage();                             // Send the ESP-NOW message to Device 2 immediately

    displayMessage("   One moment   ", "    Please!     ");  // Display "One moment" message right away
    lastMessageChangeTime = currentTime;             // Set message change time for timing next message
  }

  // If button was pressed, show a series of messages to confirm help is on the way
  if (buttonPressed) {
    // Show the next message in the sequence after the DISPLAY_DELAY
    if (currentTime - lastMessageChangeTime > DISPLAY_DELAY) {
      lastMessageChangeTime = currentTime;           // Update time for message change

      // Show different messages based on the current message index
      if (messageIndex == 0) {
        displayMessage("Someone will be", "  out shortly.  ");   // Display second message
        messageIndex = -1;                           // Set to end of sequence
      } else {
        // Reset to default state after the sequence is complete
        buttonPressed = false;                       // Reset button pressed flag
        messageIndex = 0;                            // Reset message index to initial state
      }
    }
  } else {
    // Display the default messages when no button press is detected
    if (currentTime - lastMessageChangeTime > 4000) {  // Change every 4 seconds
      lastMessageChangeTime = currentTime;             // Update message time for next loop

      if (messageIndex == 0) {
        displayMessage("   Welcome to    ", "Apptech Geelong");  // Show first default message
        messageIndex = 1;                              // Move to the next message
      } else {
        displayMessage("If you need help", "Press the button");  // Show help prompt
        messageIndex = 0;                              // Loop back to the first message
      }
    }
  }
}
