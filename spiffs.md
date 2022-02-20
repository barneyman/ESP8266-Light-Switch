# SPIFFS images

find `mkspiffs.exe`

Settings were gleaned from the `boards.txt` under the `hardware` directories

## Create spiffs images

## Wemos D1 4M1M
`mkspiffs -c ./data -p 256 -b 8192 -s 0xFA000 wemosd1.img`

## Sonoff Basic 128k SPIFFS
`mkspiffs -c ./data -p 256 -b 4096 -s 0x20000 sonoffbasic.img`

## ESP32 Cam - Minimal SPIFFS
`mkspiffs -c ./data -p 256 -b 4096 -s 0x30000 esp32cam.img`


## Upload 
### Pre-requisite
`pip install esptool`

## Wemos D1 4M1M 
### spiffs image
`esptool.py --chip esp8266 --before default_reset --after hard_reset --baud 921600 --port <Serial Port> write_flash 0x300000 ./wemosd1.img`
### app image
`esptool.py --chip esp8266 --before default_reset --after hard_reset --baud 921600 --port <Serial Port> write_flash 0 ./wemosd1.bin`
### app **and ** image
`esptool.py --chip esp8266 --before default_reset --after hard_reset --baud 921600 --port <Serial Port> write_flash 0 ./wemosd1.bin 0x300000 ./wemosd1.img`


## ESP32Cam (Minimal SPIFFS)
### clean
`esptool.py --chip esp32 --port COM5 erase_flash`

#### nb
ESP32 puts down a bootloader and an OTA file so you may need to do this the first time you burn to one
```
0xe000 ...\Arduino15\packages\esp32\hardware\esp32\1.0.6/tools/partitions/boot_app0.bin 
0x1000 ...\Arduino15\packages\esp32\hardware\esp32\1.0.6/tools/sdk/bin/bootloader_qio_80m.bin 
```

### Simultaneous (unproven)
The partition table is supplied too
```
esptool.py --chip esp32 --port COM5 write_flash -fm qio 0x10000 .\build\sonoffbasic.bin 0x3D0000 ./esp32cam.img 0x8000 .build\\partitions.bin
```

### Individual
### binary image
`esptool.py --chip esp32 --port COM5 write_flash -fm qio 0x10000 .\build\sonoffbasic.bin`
### spiffs image
`esptool.py --chip esp32 --port COM5 write_flash -fm qio 0x3D0000 ./esp32cam.img`
### partition
`esptool.py --chip esp32 --port COM5 write_flash -fm qio 0x8000 .build\\partitions.bin`




## Sonoff 
### clean
`esptool.py --chip esp8266 --port COM5 erase_flash`
### Simultaneous (proven)
`esptool.py --chip esp8266 --port COM5 write_flash -fs 1MB -fm dout 0x0 .\build\sonoffbasic.bin 0xDB000 ./sonoffbasic.img`

### Individual
### binary image
`esptool.py --chip esp8266 --port COM5 write_flash -fs 1MB -fm dout 0x0 .\build\sonoffbasic.bin`
### spiffs image
`esptool.py --chip esp8266 --port COM5 write_flash -fm dout 0xDB000 ./sonoffbasic.img`





## Docker

There is a docker image with esptool.py and mkspiffs in it, running under ubuntu

### build the packer / uploader image
```
docker build -f Dockerfile.esptools -t esptools:latest .
```


### ESP32 Cam - Minimal SPIFFS

#### create spiffs - proven
```
docker run --rm -v F:\dev\esp8266\lightswitch\lightswitch\data.vue:/in -v F:\dev\esp8266\lightswitch\lightswitch\images:/out esptools:latest ./mkspiffs_esp32 -c /in -p 256 -b 4096 -s 0x30000 /out/esp32cam.img
```


### Wemos D1 4M1M
#### create spiffs - proven
```
docker run --rm -v F:\dev\esp8266\lightswitch\lightswitch\data.vue:/in -v F:\dev\esp8266\lightswitch\lightswitch\images:/out esptools:latest ./mkspiffs+esp8266 -c /in -b 8192 -p 256 -s 0xFA000 /out/wemosd1.img
```

#### upload spiffs **and** image
#### upload spiffs **only**
```
docker run --rm --device=COM3:/dev/ttyUSB0 -v F:\dev\esp8266\lightswitch\lightswitch\images:/in esptools:latest esptool.py --chip esp8266 --before default_reset --after hard_reset --baud 921600 --port /dev/ttyUSB0 write_flash 0x300000 /in/wemosd1.img
```


### create spiffs image
