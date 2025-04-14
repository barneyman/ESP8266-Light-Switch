

all: sonoff wemosd1 esp32cam esp32wroom
spiffs: sonoff_spiffs wemosd1_spiffs esp32cam_spiffs esp32wroom_spiffs

SONOFF_FQBN="esp8266:esp8266:generic:xtal=80,vt=iram,exception=disabled,stacksmash=disabled,ssl=all,mmu=3232,non32xfer=fast,ResetMethod=ck,CrystalFreq=26,FlashFreq=40,FlashMode=dout,eesz=1M128,led=13,sdk=nonosdk_190703,ip=lm2f,dbg=Disabled,lvl=None____,wipe=none,baud=115200"
SONOFF_FRIENDLY=sonoff_basic
SONOFF_SPIFFS=mkspiffs_esp8266
SONOFF_SPIFFS_OPTS=-p 256 -b 4096 -s 0x20000

WEMOSD1_FQBN="esp8266:esp8266:d1_mini:xtal=80,vt=iram,exception=disabled,ssl=all,eesz=4M1M,ip=lm2f,dbg=Disabled"
WEMOSD1_FRIENDLY=wemosD1
WEMOSD1_SPIFFS=mkspiffs_esp8266
WEMOSD1_SPIFFS_OPTS=-p 256 -b 8192 -s 0xFA000

ESP32CAM_FQBN="esp32:esp32:esp32wrover:PartitionScheme=huge_app,FlashMode=qio,FlashFreq=40,UploadSpeed=115200,DebugLevel=none"
ESP32CAM_FRIENDLY=esp32_cam
ESP32CAM_SPIFFS=mkspiffs_esp32
ESP32CAM_SPIFFS_OPTS=-p 256 -b 4096 -s 0xF0000 # REALLY important that the SPIFFS image is the same size as the partition

ESP32WROOM_FQBN="esp32:esp32:uPesy_wroom:PartitionScheme=huge_app,CPUFreq=240,UploadSpeed=921600,FlashMode=qio,FlashFreq=40,DebugLevel=none"
ESP32WROOM_FRIENDLY=esp32_wroom
ESP32WROOM_SPIFFS=mkspiffs_esp32
ESP32WROOM_SPIFFS_OPTS=-p 256 -b 4096 -s 0xF0000 # REALLY important that the SPIFFS image is the same size as the partition


ESP8266MKSPFISS=~/.arduino15/packages/esp8266/tools/mkspiffs/3.0.4-gcc10.3-1757bed/mkspiffs
ESP32MKSPIFFS=~/.arduino15/packages/esp32/tools/mkspiffs/0.2.3/mkspiffs

sonoff: sonoff_spiffs
	- mkdir ./build
	- mkdir ./build/$(SONOFF_FRIENDLY)
	arduino-cli compile --fqbn $(SONOFF_FQBN) --output-dir ./build/$(SONOFF_FRIENDLY) --build-property compiler.cpp.extra_flags="-D_VERSION_FRIENDLY_CLI=$(SONOFF_FRIENDLY)"  --libraries ./libraries ESP8266-Light-Switch
	mv ./build/$(SONOFF_FRIENDLY)/ESP8266-Light-Switch.ino.bin ./build/$(SONOFF_FRIENDLY)/$(SONOFF_FRIENDLY).bin

sonoff_spiffs:
	- mkdir ./build
	- mkdir ./build/$(SONOFF_FRIENDLY)
	$(ESP8266MKSPFISS) -c ./data $(SONOFF_SPIFFS_OPTS) ./build/$(SONOFF_FRIENDLY)/$(SONOFF_FRIENDLY).spiffs

wemosd1: wemosd1_spiffs
	- mkdir ./build
	- mkdir ./build/$(WEMOSD1_FRIENDLY)
	arduino-cli compile --fqbn $(WEMOSD1_FQBN) --output-dir ./build/$(WEMOSD1_FRIENDLY) --build-property compiler.cpp.extra_flags="-D_VERSION_FRIENDLY_CLI=$(WEMOSD1_FRIENDLY)"  --libraries ./libraries ESP8266-Light-Switch	
	mv ./build/$(WEMOSD1_FRIENDLY)/ESP8266-Light-Switch.ino.bin ./build/$(WEMOSD1_FRIENDLY)/$(WEMOSD1_FRIENDLY).bin

wemosd1_spiffs:
	- mkdir ./build
	- mkdir ./build/$(WEMOSD1_FRIENDLY)
	$(ESP8266MKSPFISS) -c ./data $(WEMOSD1_SPIFFS_OPTS) ./build/$(WEMOSD1_FRIENDLY)/$(WEMOSD1_FRIENDLY).spiffs

esp32cam: esp32cam_spiffs
	- mkdir ./build
	- mkdir ./build/$(ESP32CAM_FRIENDLY)
	arduino-cli compile --fqbn $(ESP32CAM_FQBN) --output-dir ./build/$(ESP32CAM_FRIENDLY) --build-property compiler.cpp.extra_flags="-D_VERSION_FRIENDLY_CLI=$(ESP32CAM_FRIENDLY) -D_ESP32CAM"  --libraries ./libraries ESP8266-Light-Switch	
	mv ./build/$(ESP32CAM_FRIENDLY)/ESP8266-Light-Switch.ino.bin ./build/$(ESP32CAM_FRIENDLY)/$(ESP32CAM_FRIENDLY).bin

esp32cam_spiffs:
	- mkdir ./build
	- mkdir ./build/$(ESP32CAM_FRIENDLY)
	$(ESP32MKSPIFFS) -c ./data $(ESP32CAM_SPIFFS_OPTS) ./build/$(ESP32CAM_FRIENDLY)/$(ESP32CAM_FRIENDLY).spiffs


esp32wroom: esp32wroom_spiffs
	- mkdir ./build
	- mkdir ./build/$(ESP32WROOM_FRIENDLY)
	arduino-cli compile --fqbn $(ESP32WROOM_FQBN) --output-dir ./build/$(ESP32WROOM_FRIENDLY) \
		--build-property compiler.cpp.extra_flags="-D_VERSION_FRIENDLY_CLI=$(ESP32WROOM_FRIENDLY)" \
		--libraries ./libraries ESP8266-Light-Switch	
	mv ./build/$(ESP32WROOM_FRIENDLY)/ESP8266-Light-Switch.ino.bin ./build/$(ESP32WROOM_FRIENDLY)/$(ESP32WROOM_FRIENDLY).bin

esp32wroom_spiffs:
	- mkdir ./build
	- mkdir ./build/$(ESP32WROOM_FRIENDLY)
	$(ESP32MKSPIFFS) -c ./data $(ESP32WROOM_SPIFFS_OPTS) ./build/$(ESP32WROOM_FRIENDLY)/$(ESP32WROOM_FRIENDLY).spiffs


burn_sonoff_spiffs: sonoff_spiffs
	esptool.py --chip esp8266 --port /dev/ttyACM0 write_flash -fm dout 0xDB000 ./build/$(SONOFF_FRIENDLY)/$(SONOFF_FRIENDLY).spiffs

burn_sonoff_bin: sonoff
	esptool.py --chip esp8266 --port /dev/ttyACM0 write_flash -fs 1MB -fm dout 0x0 ./build/$(SONOFF_FRIENDLY)/$(SONOFF_FRIENDLY).bin

burn_wemosd1_spiffs: wemosd1_spiffs
	esptool.py --chip esp8266 --before default_reset --after hard_reset --baud 921600 --port /dev/ttyUSB0 write_flash 0x300000 ./build/$(WEMOSD1_FRIENDLY)/$(WEMOSD1_FRIENDLY).spiffs

burn_wemosd1_bin: wemosd1
	esptool.py --chip esp8266 --before default_reset --after hard_reset --baud 921600 --port /dev/ttyUSB0 write_flash 0 ./build/$(WEMOSD1_FRIENDLY)/$(WEMOSD1_FRIENDLY).bin

burn_wemosd1: burn_wemosd1_spiffs burn_wemosd1_bin

# ~/.arduino15/packages/esp32/hardware/esp32/3.2.0/boards.txt 
# and ~/.arduino15/packages/esp32/hardware/esp32/3.2.0/tools/partitions
# for clues - using huge_app.csv
#
# # Name,   Type, SubType, Offset,  Size, Flags
# nvs,      data, nvs,     0x9000,  0x5000,
# otadata,  data, ota,     0xe000,  0x2000,
# app0,     app,  ota_0,   0x10000, 0x300000,
# spiffs,   data, spiffs,  0x310000,0xE0000,
# coredump, data, coredump,0x3F0000,0x10000,

# bootloader goes at 0x1000
# partitions at 0x8000
# I assume boot_app0 is queried by the bootloader and jumps to 
#  	0x10000 as a result - it's the mechanism the OTA A/B partition works



# bin and spiffs
burn_esp32wroom_cold: esp32wroom
	esptool.py --chip esp32 -b 460800 --before default_reset --after hard_reset \
		--port /dev/ttyUSB0 \
		write_flash \
		--flash_mode dio --flash_size 4MB --flash_freq 40m \
		0x1000 ./build/$(ESP32WROOM_FRIENDLY)/ESP8266-Light-Switch.ino.bootloader.bin \
		0x8000 ./build/$(ESP32WROOM_FRIENDLY)/ESP8266-Light-Switch.ino.partitions.bin \
		0x10000 ./build/$(ESP32WROOM_FRIENDLY)/esp32_wroom.bin \
		0x310000 ./build/$(ESP32WROOM_FRIENDLY)/esp32_wroom.spiffs \
		0xe000 ~/.arduino15/packages/esp32/hardware/esp32/2.0.2/tools/partitions/boot_app0.bin


burn_esp32wroom: esp32wroom
	esptool.py --chip esp32 -b 460800 --before default_reset --after hard_reset \
		--port /dev/ttyUSB0 \
		write_flash \
		--flash_mode dio --flash_size 4MB --flash_freq 40m \
		0x10000 ./build/$(ESP32WROOM_FRIENDLY)/esp32_wroom.bin 


burn_esp32wroom_and_spiffs: esp32wroom
	esptool.py --chip esp32 -b 460800 --before default_reset --after hard_reset \
		--port /dev/ttyUSB0 \
		write_flash \
		--flash_mode dio --flash_size 4MB --flash_freq 40m \
		0x10000 ./build/$(ESP32WROOM_FRIENDLY)/esp32_wroom.bin \
		0x310000 ./build/$(ESP32WROOM_FRIENDLY)/esp32_wroom.spiffs 


burn_esp32cam_cold: esp32cam
	esptool.py --chip esp32 -b 460800\
		--port /dev/ttyUSB0 \
		write_flash \
		--flash_mode dio --flash_size 4MB --flash_freq 40m \
		0x1000 ./build/$(ESP32CAM_FRIENDLY)/ESP8266-Light-Switch.ino.bootloader.bin \
		0x8000 ./build/$(ESP32CAM_FRIENDLY)/ESP8266-Light-Switch.ino.partitions.bin \
		0x10000 ./build/$(ESP32CAM_FRIENDLY)/esp32_cam.bin \
		0x310000 ./build/$(ESP32CAM_FRIENDLY)/esp32_cam.spiffs \
		0xe000 ~/.arduino15/packages/esp32/hardware/esp32/2.0.2/tools/partitions/boot_app0.bin


burn_esp32cam: esp32cam
	esptool.py --chip esp32 -b 460800 \
		--port /dev/ttyUSB0 \
		write_flash \
		--flash_mode dio --flash_size 4MB --flash_freq 40m \
		0x10000 ./build/$(ESP32CAM_FRIENDLY)/esp32_cam.bin 


burn_esp32cam_and_spiffs: esp32cam
	esptool.py --chip esp32 -b 460800\
		--port /dev/ttyUSB0 \
		write_flash \
		--flash_mode dio --flash_size 4MB --flash_freq 40m \
		0x310000 ./build/$(ESP32CAM_FRIENDLY)/esp32_cam.spiffs \
		0x10000 ./build/$(ESP32CAM_FRIENDLY)/esp32_cam.bin 


clean:
	rm -r ./build/*
	
