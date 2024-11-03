# FrontCounter-Buzzer

## Overview

FrontCounter-Buzzer is an ESP32-based customer alert system designed for small businesses. This project consists of two ESP32 devices:

- **Device 1 (Front Counter)**: Equipped with a button and an LCD display. It sits on the counter and, when pressed, sends a signal via ESP-NOW to Device 2.
- **Device 2 (Back Room)**: Features a buzzer, NeoPixel LEDs, a DHT sensor for temperature and humidity monitoring, and an OLED display. When it receives a signal, it triggers an alert to notify back-room staff.

With a press of the button on Device 1, the system sends a notification that activates visual and audible alerts on Device 2, letting back-room staff know a customer needs assistance.

## Features

- **Wireless Communication**: Uses ESP-NOW for fast, reliable communication between devices.
- **Customer Notification**: Customers press a button on Device 1 to request assistance, triggering alerts on Device 2.
- **Visual and Audible Alerts**: Device 2 sounds a buzzer, displays "CUSTOMER SERVICE!!!" on the OLED screen, and flashes the NeoPixels.
- **Environmental Monitoring**: Device 2 periodically displays temperature and humidity information from a DHT sensor.
- **Alert Silencing Button**: Device 2 has a button that staff can press to silence the alert.

## Hardware Requirements

- 2 x ESP32 boards
- 1 x 16x2 I2C LCD display (Device 1)
- 1 x SSD1306 OLED display (Device 2)
- 1 x DHT11 temperature and humidity sensor (Device 2)
- 1 x NeoPixel strip (8 LEDs) (Device 2)
- 2 x Buttons (1 each for Device 1 and Device 2)
- 1 x Buzzer (Device 2)
- Jumper wires and breadboards

## Wiring Diagrams

### Device 1: Front Counter Button

| Component          | ESP32 Pin | Description                   |
|--------------------|-----------|-------------------------------|
| Button             | GPIO 3    | Customer button               |
| LCD SDA (I2C Data) | GPIO 7    | I2C data connection for LCD   |
| LCD SCL (I2C Clock)| GPIO 8    | I2C clock connection for LCD  |

**Wiring Details:**

- **Button:** Connect one leg to GPIO 3 and the other to GND.
- **LCD Display (I2C):** Connect SDA to GPIO 7 and SCL to GPIO 8. Connect VCC and GND to the ESP32’s 3.3V and GND pins.

### Device 2: Back Room Alert

| Component            | ESP32 Pin | Description                             |
|----------------------|-----------|-----------------------------------------|
| OLED SDA (I2C Data)  | GPIO 21   | I2C data connection for OLED           |
| OLED SCL (I2C Clock) | GPIO 22   | I2C clock connection for OLED          |
| DHT Sensor           | GPIO 33   | Temperature and humidity sensor data   |
| NeoPixel             | GPIO 32   | Data input for NeoPixel LEDs           |
| Buzzer               | GPIO 18   | Buzzer for audible alert               |
| Button               | GPIO 12   | Button to silence the alert            |

**Wiring Details:**

- **OLED Display (I2C):** SDA to GPIO 21, SCL to GPIO 22, power with 3.3V and GND.
- **DHT11 Sensor:** Data pin to GPIO 33, VCC to 3.3V, and GND to GND.
- **NeoPixel:** DIN to GPIO 32, VCC to 3.3V, and GND to GND.
- **Buzzer:** Positive terminal to GPIO 18, negative terminal to GND.
- **Silencing Button:** Connect one leg to GPIO 12 and the other to GND.

## Setup and Configuration

1. **Clone this Repository**: Download the code to your local development environment.

    ```bash
    git clone https://github.com/yourusername/FrontCounter-Buzzer.git
    ```

2. **Install Required Libraries**: In the Arduino IDE, install the following:
   - `LiquidCrystal I2C`
   - `ESP-NOW`
   - `Adafruit SSD1306`
   - `Adafruit GFX`
   - `Adafruit NeoPixel`
   - `DHT Sensor Library`

3. **Configure Device 1**:
   - Upload the code for Device 1 (`Device1.ino`) to the first ESP32.
   - Update `receiverMAC` in Device 1's code with Device 2's MAC address (find with `WiFi.macAddress()` on Device 2).

4. **Configure Device 2**:
   - Upload the code for Device 2 (`Device2.ino`) to the second ESP32.
   - Ensure Device 2 is powered and connected to the buzzer, NeoPixel, OLED, and DHT sensor.

## Code Explanation

### Device 1 Code

- **ESP-NOW Communication**: Initializes ESP-NOW and registers Device 2’s MAC address to send signals when the button is pressed.
- **Button Handling**: Uses debounce logic to avoid accidental double-presses and sends a signal to Device 2 upon a valid press.
- **LCD Display**: Displays messages to provide feedback to the customer, such as "One moment, please!" and "Someone will be out shortly."

### Device 2 Code

- **ESP-NOW Receiver**: Listens for signals from Device 1 and triggers an alert upon receipt.
- **Alert System**: Activates the buzzer, flashes the NeoPixel LEDs, and displays "CUSTOMER SERVICE!!!" on the OLED screen.
- **Environmental Monitoring**: Reads temperature and humidity data from the DHT sensor and updates the OLED screen.
- **Silence Button**: Allows staff to silence the alert, deactivating the buzzer and visual alerts.

## Usage

1. **Place Device 1** at the front counter, where customers can press the button to request assistance.
2. **Install Device 2** in the back room, connected to power. The NeoPixel and buzzer activate upon receiving a signal.
3. **Temperature and Humidity Monitoring**: Device 2 periodically displays temperature and humidity when no alerts are active.

## Example Output

When the button on Device 1 is pressed:

- **Device 1**: Displays "One moment, please!" and "Someone will be out shortly."
- **Device 2**: Sounds the buzzer, flashes the NeoPixel LEDs in a rainbow pattern, and displays "CUSTOMER SERVICE!!!" on the OLED screen.

After pressing the silence button on Device 2:

- **Device 2**: Stops the buzzer and light effects, returning to displaying temperature, humidity, and the bell counter.

## Troubleshooting

- **ESP-NOW Initialization Fails**: Ensure Wi-Fi mode is set to `WIFI_STA` on both devices.
- **No Display on LCD/OLED**: Double-check I2C wiring (SDA and SCL pins) and confirm addresses (0x27 for LCD, 0x3C for OLED).
- **DHT Sensor Not Reading**: Verify DHT pin configuration and ensure the DHT library is installed.
- **Buzzer or NeoPixel Not Working**: Confirm GPIO connections and check the power supply.

## License

This project is licensed under the MIT License. See the `LICENSE` file for details.

---

## Screenshots and Diagrams

### Device 1 (Front Counter) Wiring Diagram

    ESP32     Button        LCD Display
    --------  --------      ----------------
    GPIO 3 --> Button       SDA --> GPIO 7
                  |         SCL --> GPIO 8
                 GND       VCC --> 3.3V

### Device 2 (Back Room) Wiring Diagram

    ESP32        Button        DHT11     NeoPixel    OLED    Buzzer
    --------     --------      ------    --------    ----    ------
    GPIO 12 ---> Button        GPIO 33   GPIO 32     GPIO 21
