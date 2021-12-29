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
### spiffs image
`esptool.py --chip esp8266 --before default_reset --after hard_reset --baud 921600 --port <Serial Port> write_flash 0x300000 ./wemosd1.img`

## Sonoff (reset before each step)
### clean
`esptool.py --chip esp8266 --port COM5 erase_flash`
### Simultaneous
`esptool.py --chip esp8266 --port COM5 write_flash -fs 1MB -fm dout 0x0 .\build\sonoffbasic.bin 0xDB000 ./sonoffbasic.img`

### binary image
~~`esptool.py --chip esp8266 --port COM5 write_flash -fs 1MB -fm dout 0x0 .\build\sonoffbasic.bin`~~
### spiffs image
~~`esptool.py --chip esp8266 --port COM5 write_flash -fm dout 0xDB000 ./sonoffbasic.img`~~

