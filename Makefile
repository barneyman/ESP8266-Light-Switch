

all: sonoff wemosd1 esp32cam
spiffs: sonoff_spiffs wemosd1_spiffs esp32cam_spiffs

SONOFF_FQBN="esp8266:esp8266:generic:xtal=80,vt=iram,exception=disabled,stacksmash=disabled,ssl=all,mmu=3232,non32xfer=fast,ResetMethod=ck,CrystalFreq=26,FlashFreq=40,FlashMode=dout,eesz=1M128,led=13,sdk=nonosdk_190703,ip=lm2f,dbg=Disabled,lvl=None____,wipe=none,baud=115200"
SONOFF_FRIENDLY=sonoff_basic
SONOFF_SPIFFS=mkspiffs_esp8266
SONOFF_SPIFFS_OPTS=-p 256 -b 4096 -s 0x20000

WEMOSD1_FQBN="esp8266:esp8266:d1_mini:xtal=80,vt=iram,exception=disabled,ssl=all,eesz=4M1M,ip=lm2f,dbg=Disabled"
WEMOSD1_FRIENDLY=wemosd1
WEMOSD1_SPIFFS=mkspiffs_esp8266
WEMOSD1_SPIFFS_OPTS=-p 256 -b 8192 -s 0xFA000

ESP32CAM_FQBN="esp32:esp32:esp32wrover:PartitionScheme=min_spiffs,FlashMode=qio,FlashFreq=80,UploadSpeed=115200,DebugLevel=none"
ESP32CAM_FRIENDLY=esp32_cam
ESP32CAM_SPIFFS=mkspiffs_esp32
ESP32CAM_SPIFFS_OPTS=-p 256 -b 4096 -s 0x3D0000

ESP8266MKSPFISS=~/.arduino15/packages/esp8266/tools/mkspiffs/3.0.4-gcc10.3-1757bed/mkspiffs
ESP32MKSPIFFS=~/.arduino15/packages/esp32/tools/mkspiffs/0.2.3/mkspiffs

sonoff: sonoff_spiffs
	- mkdir ./build
	- mkdir ./build/$(SONOFF_FRIENDLY)
	arduino-cli compile --fqbn $(SONOFF_FQBN) --output-dir ./build/$(SONOFF_FRIENDLY) --build-property compiler.cpp.extra_flags="-D_VERSION_FRIENDLY_CLI=$(SONOFF_FRIENDLY)"  --libraries ./libraries ESP8266-Light-Switch
	mv ./build/sonoff/ESP8266-Light-Switch.ino.bin ./build/$(SONOFF_FRIENDLY)/$(SONOFF_FRIENDLY).bin

sonoff_spiffs:
	- mkdir ./build
	- mkdir ./build/$(SONOFF_FRIENDLY)
	$(ESP8266MKSPFISS) -c ./data $(SONOFF_SPIFFS_OPTS) ./build/$(SONOFF_FRIENDLY)/$(SONOFF_FRIENDLY).spiffs

wemosd1: wemosd1_spiffs
	- mkdir ./build
	- mkdir ./build/$(WEMOSD1_FRIENDLY)
	arduino-cli compile --fqbn $(WEMOSD1_FQBN) --output-dir ./build/$(WEMOSD1_FRIENDLY) --build-property compiler.cpp.extra_flags="-D_VERSION_FRIENDLY_CLI=$(WEMOSD1_FRIENDLY)"  --libraries ./libraries ESP8266-Light-Switch	
	mv ./build/wemosd1/ESP8266-Light-Switch.ino.bin ./build/$(WEMOSD1_FRIENDLY)/$(WEMOSD1_FRIENDLY).bin

wemosd1_spiffs:
	- mkdir ./build
	- mkdir ./build/$(WEMOSD1_FRIENDLY)
	$(ESP8266MKSPFISS) -c ./data $(WEMOSD1_SPIFFS_OPTS) ./build/$(WEMOSD1_FRIENDLY)/$(WEMOSD1_FRIENDLY).spiffs

esp32cam: esp32cam_spiffs
	- mkdir ./build
	- mkdir ./build/$(ESP32CAM_FRIENDLY)
	arduino-cli compile --fqbn $(ESP32CAM_FQBN) --output-dir ./build/$(ESP32CAM_FRIENDLY) --build-property compiler.cpp.extra_flags="-D_VERSION_FRIENDLY_CLI=$(ESP32CAM_FRIENDLY)"  --libraries ./libraries ESP8266-Light-Switch	
	mv ./build/esp32cam/ESP8266-Light-Switch.ino.bin ./build/$(ESP32CAM_FRIENDLY)/$(ESP32CAM_FRIENDLY).bin

esp32cam_spiffs:
	- mkdir ./build
	- mkdir ./build/$(ESP32CAM_FRIENDLY)
	$(ESP32MKSPIFFS) -c ./data $(ESP32CAM_SPIFFS_OPTS) ./build/$(ESP32CAM_FRIENDLY)/$(ESP32CAM_FRIENDLY).spiffs



burn_sonoff_spiffs: sonoff_spiffs
	esptool.py --chip esp8266 --port /dev/ttyUSB0 write_flash -fm dout 0xDB000 ./build/sonoff/$(SONOFF_FRIENDLY).spiffs

burn_sonoff_bin: sonoff
	esptool.py --chip esp8266 --port /dev/ttyUSB0 write_flash -fs 1MB -fm dout 0x0 ./build/sonoff/$(SONOFF_FRIENDLY).bin

burn_wemosd1_spiffs: wemosd1_spiffs
	esptool.py --chip esp8266 --before default_reset --after hard_reset --baud 921600 --port /dev/ttyUSB0 write_flash 0x300000 ./build/wemosd1/$(WEMOSD1_FRIENDLY).spiffs

burn_wemosd1_bin: wemosd1
	esptool.py --chip esp8266 --before default_reset --after hard_reset --baud 921600 --port /dev/ttyUSB0 write_flash 0 ./build/wemosd1/$(WEMOSD1_FRIENDLY).bin

burn_wemosd1: burn_wemosd1_spiffs burn_wemosd1_bin

clean:
	rm -r ./build/*
	