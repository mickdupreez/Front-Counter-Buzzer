// Device 2 Buzzer in the back of house.

#include <WiFi.h>               // WiFi library required for ESP-NOW communication
#include <esp_now.h>            // ESP-NOW library for direct ESP32 communication
#include <Wire.h>               // Wire library for I2C communication
#include <Adafruit_GFX.h>       // Adafruit graphics library for display functionality
#include <Adafruit_SSD1306.h>   // Library for SSD1306 OLED display
#include <DHT.h>                // Library for DHT temperature and humidity sensor
#include <Adafruit_NeoPixel.h>  // Library for controlling NeoPixel LED strip

// Screen dimensions and I2C address for the OLED display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS 0x3C

// Initialize the OLED display object with specified dimensions and I2C address
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// DHT sensor type and pin configuration
#define DHTTYPE DHT11           // Specify DHT11 sensor type
const int dhtPin = 33;          // GPIO pin for DHT sensor data
DHT dht(dhtPin, DHTTYPE);       // Initialize DHT sensor with pin and type

// GPIO pin definitions for button, buzzer, and NeoPixel
const int buttonPin1 = 12;      // Button 1 for silencing alerts
const int buzzerPin = 18;       // Buzzer pin
const int neopixelPin = 32;     // NeoPixel data pin
#define NUMPIXELS 16            // Number of NeoPixel LEDs

// Initialize NeoPixel object with pin, number of pixels, and configuration
Adafruit_NeoPixel pixels(NUMPIXELS, neopixelPin, NEO_GRB + NEO_KHZ800);

// Variables for alert tracking and sensor data storage
int bellCounter = 0;            // Counter for button presses from Device 1
bool alertActive = false;       // Flag to track if an alert is currently active
float lastTemperature = -100.0; // Variable to store last recorded temperature
float lastHumidity = -100.0;    // Variable to store last recorded humidity

// Define an array of vibrant colors for the light show effect
uint32_t colors[] = {
    pixels.Color(255, 0, 0),    // Red
    pixels.Color(0, 255, 0),    // Green
    pixels.Color(0, 0, 255),    // Blue
    pixels.Color(255, 255, 0),  // Yellow
    pixels.Color(0, 255, 255),  // Cyan
    pixels.Color(255, 0, 255),  // Magenta
    pixels.Color(255, 255, 255) // White
};

// Timer variables for flashing box
unsigned long flashingTimer = 0;   // Timer for flashing effect
bool flashingOn = false;           // State for flashing box on/off

// Callback function to handle received ESP-NOW data
void onDataReceive(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
    if (len == 1 && incomingData[0] == 1) {  // Check if single-byte data with value 1 received
        Serial.println("Button press received from Device 1 via ESP-NOW!");  // Log received press
        bellCounter++;                     // Increment the counter for received button presses
        triggerCustomerServiceAlert();     // Trigger customer service alert
    }
}

// Function to initialize ESP-NOW communication
void initESPNow() {
    WiFi.begin();  // Start WiFi stack without connecting to a network (required for ESP-NOW)
    if (esp_now_init() == ESP_OK) {  // Initialize ESP-NOW and check success
        Serial.println("ESP-NOW initialized successfully.");
        esp_now_register_recv_cb(onDataReceive);  // Register onDataReceive callback for incoming messages
    } else {
        Serial.println("ESP-NOW initialization failed.");
        ESP.restart();  // Restart ESP32 on failure
    }
}

// Initial setup function
void setup() {
    Serial.begin(115200);            // Start serial communication at 115200 baud
    Wire.begin(21, 22);              // Initialize I2C with SDA on GPIO 21 and SCL on GPIO 22

    // Initialize OLED display and check for success
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
        Serial.println("OLED initialization failed!");
        while (true);                  // Halt program if OLED fails to initialize
    }
    display.setRotation(2);          // Rotate display orientation 180 degrees
    display.clearDisplay();          // Clear display buffer
    display.display();               // Send buffer to the display

    // Configure pin for button and buzzer, and initialize NeoPixel strip
    pinMode(buttonPin1, INPUT);      // Button 1 as input (for silencing alerts)
    dht.begin();                     // Start DHT sensor
    pinMode(buzzerPin, OUTPUT);      // Set buzzer as output
    pixels.begin();                  // Initialize NeoPixel strip
    pixels.clear();                  // Clear any previous NeoPixel state
    pixels.show();                   // Apply cleared state

    initESPNow();                    // Initialize ESP-NOW for communication
    Serial.println("Device 2 ready to receive ESP-NOW messages.");  // Log setup completion
}

// Main loop
void loop() {
    handleButtons();                 // Check button state and handle input
    if (!alertActive) {              // Only read and display DHT data when alert is inactive
        readAndDisplayDHT();         // Read and display temperature and humidity data
    }
    
    // Handle flashing effect if alert is active
    if (alertActive && millis() - flashingTimer >= 250) { // Flash every 250ms for faster speed
        flashingOn = !flashingOn;  // Toggle flashing state
        display.drawRect(0, 0, SCREEN_WIDTH, 48, flashingOn ? SSD1306_WHITE : SSD1306_BLACK); // Draw or clear box
        display.display();          // Update display with flashing border
        flashingTimer = millis();   // Reset flashing timer
    }
}

// Function to handle button press for silencing alerts
void handleButtons() {
    if (alertActive && digitalRead(buttonPin1) == HIGH) {  // Only respond to button press if alert is active
        Serial.println("Button 1 pressed: Silencing alerts.");  // Log silencing action
        silenceAlerts();                      // Call silenceAlerts function to deactivate alerts
    }
}

// Function to silence active alerts and reset NeoPixel and buzzer
void silenceAlerts() {
    alertActive = false;              // Reset alert flag to allow normal display operation
    noTone(buzzerPin);                // Stop buzzer sound
    pixels.clear();                   // Turn off all LEDs on NeoPixel
    pixels.show();                    // Apply cleared NeoPixel state
    display.clearDisplay();           // Immediately clear the display
    display.display();                // Apply cleared display
    delay(50);                        // Brief delay to prevent display issues
    readAndDisplayDHT();              // Immediately show temperature and humidity after clearing alert
}

// Function to center text horizontally
int centerText(const char* text, int textSize) {
    int textWidth = strlen(text) * 6 * textSize;  // Each character is 6 pixels wide in default font
    return (SCREEN_WIDTH - textWidth) / 2;        // Calculate center position
}

// Function to read DHT sensor and update display with temperature, humidity, and bell counter
void readAndDisplayDHT() {
    float humidity = dht.readHumidity();       // Read humidity from DHT
    float temperature = dht.readTemperature(); // Read temperature from DHT

    display.clearDisplay();                   // Clear display buffer to prepare for fresh data

    if (isnan(humidity) || isnan(temperature)) {  // Check if DHT reading failed
        Serial.println("Failed to read from DHT sensor!");  // Log sensor error
        display.setTextSize(1);                   // Set text size for error message
        display.setTextColor(SSD1306_WHITE);      // Set text color
        display.setCursor(centerText("Sensor Error", 1), 16);  // Center "Sensor Error" text
        display.println("Sensor Error");          // Display error message
    } else {                                    // If readings are valid
        lastTemperature = temperature;            // Update last temperature
        lastHumidity = humidity;                  // Update last humidity

        // Temperature Display Section with larger font and box, adjusted text position
        String tempStr = "Temp:" + String(temperature, 1) + "C";  // Format temperature with "C" without space
        display.setTextSize(2);                   // Increase text size
        display.setTextColor(SSD1306_WHITE);      // Set text color
        display.setCursor(8, 3);                  // Move text up by 2 pixels
        display.print(tempStr);                   // Display temperature
        display.drawRect(0, 0, SCREEN_WIDTH, 22, SSD1306_WHITE); // Draw box for temperature with 2-pixel gap below

        // Humidity Display Section with larger font and box, adjusted text position
        char humStr[10];                                          // Buffer for humidity string
        snprintf(humStr, sizeof(humStr), "Hum:%.1f%%", humidity);  // Format humidity without leading space
        display.setCursor(8, 28);                                // Move text down by 1 pixel
        display.print(humStr);                                   // Display humidity
        display.drawRect(0, 24, SCREEN_WIDTH, 22, SSD1306_WHITE); // Draw box for humidity with 2-pixel gap below
    }

    // Bell Counter at the bottom of the screen in the yellow section
    char counterStr[20];
    snprintf(counterStr, sizeof(counterStr), "Bell Counter: %d", bellCounter);  // Format counter text
    display.setTextSize(1);                    // Set text size for counter
    display.setTextColor(SSD1306_WHITE);       // Set text color
    display.fillRect(0, 48, SCREEN_WIDTH, 16, SSD1306_BLACK);  // Clear area for bell counter
    display.setCursor(centerText(counterStr, 1), 52);          // Center bell counter text
    display.print(counterStr);                 // Display counter
    display.drawRect(0, 48, SCREEN_WIDTH, 16, SSD1306_WHITE);  // Draw box for bell counter

    display.display();                        // Send all buffered content to display
}

// Trigger customer service alert with Nokia tune and NeoPixel effects
void triggerCustomerServiceAlert() {
    alertActive = true;                       // Set alert active flag
    display.fillRect(0, 0, SCREEN_WIDTH, 48, SSD1306_BLACK);  // Clear top area of display
    display.setTextSize(2);                   // Set text size
    display.setTextColor(SSD1306_WHITE);      // Set text color

    // Center "CUSTOMER SERVICE!!!" text vertically and horizontally in the blue section
    display.setCursor(centerText("CUSTOMER", 2), 6);     // Centered "CUSTOMER" text
    display.print("CUSTOMER");
    display.setCursor(centerText("SERVICE!!!", 2), 30);  // Centered "SERVICE!!!" text
    display.print("SERVICE!!!");

    display.display();                        // Send to display

    playNokiaTuneWithVibrantLightshow();  // Play Nokia tune with vibrant light show once

    // After the alert finishes, revert to normal display
    silenceAlerts();                        // Clear alert, display temperature and humidity
}

// Function to play Nokia tune with vibrant light show effect
void playNokiaTuneWithVibrantLightshow() {
    // Helper function to play a tone with vibrant color effect on the NeoPixel ring
    auto playNoteWithColor = [&](int frequency, int duration) {
        if (!alertActive) return;              // Exit if alert is silenced
        tone(buzzerPin, frequency, duration * 0.9); // Play the tone, slightly shorter than duration
        for (int i = 0; i < NUMPIXELS; i++) {  // Loop through each NeoPixel
            if (!alertActive) break;           // Exit loop if alert is silenced
            int randomPixel = random(0, NUMPIXELS);   // Select random pixel
            uint32_t vibrantColor = colors[random(0, 7)];  // Choose random color
            pixels.setPixelColor(randomPixel, vibrantColor); // Set pixel color
            pixels.show();                         // Update NeoPixel ring
            delay(duration / NUMPIXELS);           // Delay to create staggered lighting effect
            pixels.setPixelColor(randomPixel, 0);   // Turn off pixel after delay
            pixels.show();                         // Update NeoPixel ring
        }
        noTone(buzzerPin);                         // Stop any sound from the buzzer
    };

    // Nokia tune sequence with synchronized light effects
    playNoteWithColor(1319, 125);
    playNoteWithColor(1175, 125);
    playNoteWithColor(740, 250);
    playNoteWithColor(831, 250);
    playNoteWithColor(1109, 125);
    playNoteWithColor(988, 125);
    playNoteWithColor(587, 250);
    playNoteWithColor(659, 250);
    playNoteWithColor(988, 125);
    playNoteWithColor(880, 125);
    playNoteWithColor(554, 250);
    playNoteWithColor(659, 250);
    playNoteWithColor(880, 750);

    pixels.clear();                              // Clear NeoPixel ring after tune
    pixels.show();                               // Update NeoPixel ring to reflect cleared state
}
