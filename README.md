# ⏰ Smart Alarm Clock (ESP32)

![ESP32](https://img.shields.io/badge/ESP32-100000?style=for-the-badge&logo=espressif&logoColor=white)
![Arduino](https://img.shields.io/badge/Arduino-00979D?style=for-the-badge&logo=arduino&logoColor=white)
![C++](https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)

A smart alarm clock based on the ESP32 microcontroller featuring a web interface, OLED display, MP3 player, sunrise simulation, and a German vocabulary learning function.

## 🌟 Features
* 🌐 **Web Interface**: Manage alarms and synchronize time directly from your browser (accessible via IP or `http://alarm.local`).
* 🌤️ **Auto-Weather & Location**: The clock automatically detects its physical location via IP address and fetches current weather and wind speed using the *Open-Meteo API*.
* 🎵 **MP3 Alarm**: Plays tracks from a flash drive via the JQ6500 module (configure up to 5 different alarms for specific days of the week with selectable melodies).
* 🌅 **Sunrise Simulation**: Controls a 5V relay (220V) to automatically turn on a bedside lamp when the alarm rings.
* 🇩🇪 **Language Learning**: When the alarm goes off, a random German word with its translation and difficulty level appears on the screen to help you memorize it.
* 🌗 **Smart Brightness**: Smooth display brightness transitions (sunrise/sunset) and a low-power night mode to prevent OLED coil whine.
* 💾 **Non-Volatile Memory**: All alarm settings are safely stored in the ESP32 flash memory and persist even after power loss (e.g., when running on a Powerbank).
* 🔄 **OTA Updates**: Supports Over-The-Air firmware updates via Wi-Fi without needing a USB cable.

## 🛠️ Hardware
* **ESP32** (Main controller)
* **OLED Display** 1.3" / 0.96" (I2C, SSD1306 / SH1106)
* **DS3231** (Real-Time Clock module)
* **JQ6500** (MP3 module with serial interface)
* **Speaker** (8Ω / 3W, connected directly to the JQ6500 built-in amplifier)
* **5V Relay** (with optocoupler, for bedside light control)
* Push button (to stop the alarm)

## 📦 Dependencies
The following libraries are required to compile the project in the Arduino IDE:
* `WiFi`, `WebServer`, `HTTPClient`, `ESPmDNS`, `Preferences` (Built into the ESP32 core)
* [U8g2](https://github.com/olikraus/u8g2) (For OLED display)
* [RTClib](https://github.com/adafruit/RTClib) (For the DS3231 RTC module)
* [ArduinoJson](https://github.com/bblanchon/ArduinoJson) (For parsing weather and API data)
* [JQ6500_Serial](https://github.com/sleemanj/JQ6500_Serial) (For the audio module)

## 🚀 Installation & Setup
1. Clone the repository:
   ```bash
   git clone [https://github.com/RuslanBaatyrbekov/esp32-smart-alarm.git](https://github.com/RuslanBaatyrbekov/esp32-smart-alarm.git)

   
Open the project in the Arduino IDE.

Add your German vocabulary dictionary to the words.h file (using the Word structure format).

Configure your Wi-Fi credentials in the setup() function:

C++
wifiMulti.addAP("YOUR_SSID", "YOUR_PASSWORD");
Flash the ESP32.

Access the web interface via the device's IP address, sync the time, and set your alarms!

📸 Interface
The project includes a responsive Dark-Mode web interface served directly from the ESP32. It allows you to easily configure days of the week, sunrise settings, and melodies from any device.

Developed for personal use and to explore IoT capabilities with the ESP32.

<img width="720" height="1280" alt="image" src="https://github.com/user-attachments/assets/776719ba-2c34-4544-9769-c9aacfcef627" />
<img width="720" height="1280" alt="image" src="https://github.com/user-attachments/assets/a929fffb-74a0-4db0-b26d-5c019abb2fcc" />
<img width="721" height="1280" alt="image" src="https://github.com/user-attachments/assets/0aeced12-f1e5-4500-82fa-c868fdb0c974" />
<img width="1280" height="994" alt="image" src="https://github.com/user-attachments/assets/0c3efc99-17d4-44af-8482-94735591d41d" />


