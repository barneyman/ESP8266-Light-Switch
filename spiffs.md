# SPIFFS images

find `mkspiffs.exe`

Settings were gleaned from the `boards.txt` under the `hardware` directories

## Create spiffs images

## Wemos D1 4M1M
`mkspiffs -c ./data -b 8192 -p 256 -s 0xFA000 wemosd1.img`

## Sonoff Basic
`mkspiffs -c ./data -p 256 -b 4096 -s 0x20000 sonoffbasic.img`

## ESP32 Cam
`mkspiffs -c ./data -p 256 -b 4096 -s 0x3D0000 esp32cam.img`


## Upload 
### Pre-requisite
`pip install esptool`

## Wemos D1 4M1M
`esptool.py --chip esp8266 --before default_reset --after hard_reset --baud 921600 --port <Serial Port> write_flash 0x300000 ./wemosd1.img`

