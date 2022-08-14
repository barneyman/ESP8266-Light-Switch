## Wemos D1 4M1M
./mkspiffs_esp8266 -c /data -p 256 -b 8192 -s 0xFA000 wemosd1.spiffs

## Sonoff Basic 128k SPIFFS
./mkspiffs_esp8266 -c /data -p 256 -b 4096 -s 0x20000 sonoffbasic.spiffs

## ESP32 Cam - Minimal SPIFFS
./mkspiffs_esp32 -c /data -p 256 -b 4096 -s 0x30000 esp32cam.spiffs


mv *.spiffs /data/