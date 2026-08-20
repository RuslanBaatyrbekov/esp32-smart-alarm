# ⏰ Smart Alarm Clock (ESP32)

![ESP32](https://img.shields.io/badge/ESP32-100000?style=for-the-badge&logo=espressif&logoColor=white)
![Arduino](https://img.shields.io/badge/Arduino-00979D?style=for-the-badge&logo=arduino&logoColor=white)
![C++](https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)

A feature-rich smart alarm clock powered by an ESP32 microcontroller, featuring a responsive dark-mode web interface, OLED display, MP3 player, sunrise simulation, train schedule integration, and a German vocabulary learning function.

## 🌟 Features

* **🌐 Web Interface:** Manage alarms, synchronize time, toggle the bedside lamp, display custom messages, and trigger schedules directly from your browser (accessible via IP or `http://alarm.local`).
* **🚆 Train Schedule Integration:** Fetch and display live train schedules from a local server endpoint, with manual "Show Now" / "Hide" controls and a master enable/disable toggle.
* **💬 Custom Screen Messaging:** Send text messages directly from the web admin panel to the OLED screen with automatic multi-line UTF-8 wrapping.
* **💡 Manual Lamp Control:** Press the physical button when the alarm is not ringing to manually toggle the bedside lamp (relays state syncs with the web interface).
* **🌤️ Auto-Weather & Location:** Automatically detects physical location via IP and fetches current weather/wind speed using the Open-Meteo API.
* **🎵 MP3 Alarm:** Plays tracks from a flash drive via the JQ6500 module (configure up to 5 different alarms for specific weekdays with selectable melodies).
* **📈 Volume Stair-Step:** Gradually increases alarm volume across stages (30% $\rightarrow$ 60% $\rightarrow$ 100%) for a comfortable wake-up experience.
* **🌅 Sunrise Simulation:** Controls a 5V relay to automatically turn on a bedside lamp when the alarm triggers.
* **🇩🇪 Language Learning:** Displays a random German word with its translation and difficulty level on the screen upon waking up.
* **🌗 Smart Brightness:** Smooth display contrast transitions (sunrise/sunset) and a low-power night mode to prevent OLED coil whine.
* **💾 Non-Volatile Memory:** All alarm configurations and settings are safely stored in flash memory using `Preferences` and persist through reboots.
* **🔄 OTA Updates:** Supports Over-The-Air firmware updates over Wi-Fi without needing a USB cable.

---

## 🛠️ Hardware Requirements

* **ESP32** (Main controller)
* **OLED Display** 1.3" / 0.96" (I2C, SSD1306 / SH1106)
* **DS3231** (Real-Time Clock module)
* **JQ6500** (MP3 module with serial interface)
* **Speaker** (8$\Omega$ / 3W, connected directly to the JQ6500 built-in amplifier)
* **5V Relay** (with optocoupler, for bedside light control)
* **Push button** (to stop the alarm or toggle the lamp)

---

## 📦 Dependencies

The following libraries are required to compile the project in the Arduino IDE:
* `WiFi`, `WebServer`, `HTTPClient`, `ESPmDNS`, `Preferences` (Built into the ESP32 core)
* **U8g2** (For OLED display rendering with Cyrillic support)
* **RTClib** (For the DS3231 RTC module)
* **ArduinoJson** (For parsing weather, train schedules, and API data)
* **JQ6500_Serial** (For the audio playback module)

---

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

<img width="2560" height="1441" alt="image" src="https://github.com/user-attachments/assets/7ea9e089-2909-443a-a0b5-0bb22fbc1f9a" />
<img width="2560" height="1441" alt="image" src="https://github.com/user-attachments/assets/f613e1cb-6786-42eb-971d-5fd297ecd1eb" />
<img width="2560" height="1441" alt="image" src="https://github.com/user-attachments/assets/116450a9-5230-4f53-8255-6529b47979c0" />
<img width="1441" height="2560" alt="image" src="https://github.com/user-attachments/assets/ff8d1f87-184b-47aa-99c1-773bd743768d" />
<img width="720" height="1280" alt="image" src="https://github.com/user-attachments/assets/776719ba-2c34-4544-9769-c9aacfcef627" />
<img width="1280" height="994" alt="image" src="https://github.com/user-attachments/assets/0c3efc99-17d4-44af-8482-94735591d41d" />


