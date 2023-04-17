

all: sonoff wemosd1 esp32cam


SONOFF_FQBN="esp8266:esp8266:generic:xtal=80,vt=iram,exception=disabled,stacksmash=disabled,ssl=all,mmu=3232,non32xfer=fast,ResetMethod=ck,CrystalFreq=26,FlashFreq=40,FlashMode=dout,eesz=1M128,led=13,sdk=nonosdk_190703,ip=lm2f,dbg=Disabled,lvl=None____,wipe=none,baud=115200"
SONOFF_FRIENDLY=sonoff_basic
SONOFF_SPIFFS=mkspiffs_esp8266
SONOFF_SPIFFS_OPTS=-p 256 -b 4096 -s 0x20000

WEMOSD1_FQBN="esp8266:esp8266:d1_mini:xtal=80,vt=iram,exception=disabled,ssl=all,eesz=4M1M,ip=lm2f,dbg=Disabled"
WEMOSD1_FRIENDLY=wemosD1
WEMOSD1_SPIFFS=mkspiffs_esp8266
WEMOSD1_SPIFFS_OPTS=-p 256 -b 8192 -s 0xFA000

ESP32CAM_FQBN="esp32:esp32:esp32wrover:PartitionScheme=min_spiffs,FlashMode=qio,FlashFreq=80,UploadSpeed=115200,DebugLevel=none"
ESP32CAM_FRIENDLY=esp32_cam
ESP32CAM_SPIFFS=mkspiffs_esp32
ESP32CAM_SPIFFS_OPTS=-p 256 -b 4096 -s 0x3D0000


sonoff: sonoff_spiffs
	- mkdir ./build
	- mkdir ./build/sonoff
	arduino-cli compile --fqbn $(SONOFF_FQBN) --output-dir ./build/sonoff --build-property compiler.cpp.extra_flags="-D_VERSION_FRIENDLY_CLI=$(SONOFF_FRIENDLY) -D_VERSION_NUM_CLI=1.1"  --libraries ./libraries ESP8266-Light-Switch

sonoff_spiffs:
	- mkdir ./build
	- mkdir ./build/sonoff
	/spiffs/$(SONOFF_SPIFFS) -c ./data $(SONOFF_SPIFFS_OPTS) ./build/sonoff/$(SONOFF_FRIENDLY).spiffs

wemosd1: wemosd1_spiffs
	- mkdir ./build
	- mkdir ./build/wemosd1
	arduino-cli compile --fqbn $(WEMOSD1_FQBN) --output-dir ./build/wemosd1 --build-property compiler.cpp.extra_flags="-D_VERSION_FRIENDLY_CLI=$(WEMOSD1_FRIENDLY) -D_VERSION_NUM_CLI=1.1"  --libraries ./libraries ESP8266-Light-Switch	

wemosd1_spiffs:
	- mkdir ./build
	- mkdir ./build/wemosd1
	/spiffs/$(WEMOSD1_SPIFFS) -c ./data $(WEMOSD1_SPIFFS_OPTS) ./build/wemosd1/$(WEMOSD1_FRIENDLY).spiffs

esp32cam: esp32cam_spiffs
	- mkdir ./build
	- mkdir ./build/esp32cam
	arduino-cli compile --fqbn $(ESP32CAM_FQBN) --output-dir ./build/esp32cam --build-property compiler.cpp.extra_flags="-D_VERSION_FRIENDLY_CLI=$(ESP32CAM_FRIENDLY) -D_VERSION_NUM_CLI=1.1"  --libraries ./libraries ESP8266-Light-Switch	

esp32cam_spiffs:
	- mkdir ./build
	- mkdir ./build/wemosd1
	/spiffs/$(ESP32CAM_SPIFFS) -c ./data $(ESP32CAM_SPIFFS_OPTS) ./build/esp32cam/$(ESP32CAM_FRIENDLY).spiffs

clean:
	rm -r ./build
	