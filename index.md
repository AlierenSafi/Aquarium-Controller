# Aquarium Controller
 This project was born out of wanting to make my aquarium smart.

Many devices were working in my aquarium and I wanted to control them all remotely. For this reason, I designed an arduino based PCB that will control them and at the same time transmit data about my aquarium to phone.

### Features of PCB:
- Internal power supply.
- 8 x relay control.
- Bluetooth.
- Wi-fi.
- 12V Fan.
- RGB LED.
- Internal RTC circuit.
- 2 x ds18b20 ports for outdoor temperature and water temperature.
- IR receiver for remote control. (optional)
- Internal buzzer for audible warnings.
- I2C output. (For LCD or another circuit which using I2C)
- Card software can be uploaded over wifi. (OTA Support) (esp-link)



### Materials:
- 1x Arduino MEGA 2560.	
- 1x LM2596 Buck converter module. [here](https://www.amazon.com/Adjustable-Converter-1-5-35v-Efficiency-Regulator/dp/B07QKHR6PY/ref=sr_1_3?dchild=1&keywords=LM2596&qid=1612530712&sr=8-3)
- 1x ESP-01 Wi-Fi module.
- 1x HC-05 Bluetooth module [here](https://www.amazon.com/HiLetgo-Bluetooth-Transceiver-Integrated-Communication/dp/B07VL725T8/ref=sr_1_2?dchild=1&keywords=hc05&qid=1612538199&sr=8-2)
- 4x 5v Relay Module (x2 pieces) [here](https://www.amazon.com/JBtek-Channel-Module-Arduino-Raspberry/dp/B00KTEN3TM/ref=sr_1_3?dchild=1&keywords=4+channel+relay+module&qid=1612532646&sr=8-3)
- 1x LM1117T-3.3V - TO-220 [here](https://www.amazon.com/Bridgold-LM1117T-3-3V-Dropout-Positive-Regulator/dp/B08C7VSCC5/ref=sr_1_2?dchild=1&keywords=LM1117&qid=1612533033&sr=8-2)
- 2x LED
- 1x TinyRTC Module [here](https://www.amazon.com/Ximimark-DS1307-AT24C32-Module-Arduino/dp/B07TVMVDDP/ref=sr_1_3?crid=3DNAK0EK2HA4J&dchild=1&keywords=tinyrtc&qid=1612533151&sprefix=tinyrt%2Caps%2C284&sr=8-3)
- 3x Push button 
- 4x IRF1010E - TO-220 - for LED and Fan kontrol [here](https://www.amazon.com/Calvas-original-IRF1010E-IRF1010EPBF-channel/dp/B07W5TX5FM/ref=sr_1_1?dchild=1&keywords=%C4%B1rf1010e&qid=1612533351&sr=8-1)
- 1x ds18b20 for outside temperature [here](https://www.amazon.com/DS18B20-Temperature-Raspberry-Separate-Antistatic/dp/B07STT9H74/ref=sr_1_5?dchild=1&keywords=ds18b20&qid=1612533528&sr=8-5)
- 1x ds18b20 for inside aquarium [here](https://www.amazon.com/DS18B20-Temperature-Waterproof-Stainless-Raspberry/dp/B087JQ6MCP/ref=sr_1_4?dchild=1&keywords=ds18b20&qid=1612533584&sr=8-4)
- 1x IR Receiver [here](https://www.amazon.com/Gikfun-Infrared-Emission-Receiver-Arduino/dp/B06XYNDRGF/ref=sr_1_3?crid=38JTZSPJBZ5N&dchild=1&keywords=ir+receiver+sensor&qid=1612533640&sprefix=ir+receiver+se%2Caps%2C302&sr=8-3)
- Resistors (11x 10k ohm, 4x 4.7k ohm, 6x 220 ohm, 1x 3.3k ohm and 1x 1k ohm)
- Capasitors (1x 100uF and 1x 10uF)
- **Suitable sockets and cables for connections.**

![Image](https://github.com/AlierenSafi/AquariumController/blob/main/Gerber%20File/List.PNG?raw=true)

### PCB Gerber File:
- You can order PCB using the gerber file.
 <img src="https://github.com/AlierenSafi/AquariumController/blob/main/Gerber%20File/pcb.PNG?raw=true" width="200" height="200">

### PCB
 <img src="https://github.com/AlierenSafi/Aquarium-Controller/blob/main/images/front.jpg?raw=true" width="200" height="200">  <img src="https://github.com/AlierenSafi/Aquarium-Controller/blob/main/images/back.jpg?raw=true" width="200" height="200">  <img src="https://github.com/AlierenSafi/Aquarium-Controller/blob/main/images/with_components.jpg?raw=true" width="200" height="200">


### ESP8266
- ESP8266 need [esp-link](https://github.com/jeelabs/esp-link) firmware.

### License
- You can use, modify, fork and redistribute this code free of charge with no restrictions for non-commercial use only. For commercial use please contact with the author.
