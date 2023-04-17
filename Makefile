

all: sonoff wemosd1 esp32cam


SONOFF_FQBN="esp8266:esp8266:generic:xtal=80,vt=iram,exception=disabled,stacksmash=disabled,ssl=all,mmu=3232,non32xfer=fast,ResetMethod=ck,CrystalFreq=26,FlashFreq=40,FlashMode=dout,eesz=1M128,led=13,sdk=nonosdk_190703,ip=lm2f,dbg=Disabled,lvl=None____,wipe=none,baud=115200"
SONOFF_FRIENDLY="sonoff_basic"

WEMOSD1_FQBN="esp8266:esp8266:d1_mini:xtal=80,vt=iram,exception=disabled,ssl=all,eesz=4M1M,ip=lm2f,dbg=Disabled"
WEMOSD1_FRIENDLY="wemosD1"

ESP32CAM_FQBN="esp32:esp32:esp32wrover:PartitionScheme=min_spiffs,FlashMode=qio,FlashFreq=80,UploadSpeed=115200,DebugLevel=none"
ESP32CAM_FRIENDLY="esp32_cam"


sonoff:
	- mkdir ./build
	- mkdir ./build/sonoff
	arduino-cli compile --fqbn $(SONOFF_FQBN) --output-dir ./build/sonoff --build-property compiler.cpp.extra_flags="-D_VERSION_FRIENDLY_CLI=$(SONOFF_FRIENDLY) -D_VERSION_NUM_CLI=1.1"  --libraries ./libraries ESP8266-Light-Switch

wemosd1:
	- mkdir ./build
	- mkdir ./build/wemosd1
	arduino-cli compile --fqbn $(WEMOSD1_FQBN) --output-dir ./build/wemosd1 --build-property compiler.cpp.extra_flags="-D_VERSION_FRIENDLY_CLI=$(WEMOSD1_FRIENDLY) -D_VERSION_NUM_CLI=1.1"  --libraries ./libraries ESP8266-Light-Switch	

esp32cam:	
	- mkdir ./build
	- mkdir ./build/esp32cam
	arduino-cli compile --fqbn $(ESP32CAM_FQBN) --output-dir ./build/esp32cam --build-property compiler.cpp.extra_flags="-D_VERSION_FRIENDLY_CLI=$(ESP32CAM_FRIENDLY) -D_VERSION_NUM_CLI=1.1"  --libraries ./libraries ESP8266-Light-Switch	

clean:
	rm -r ./build
	