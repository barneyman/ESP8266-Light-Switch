arduino-cli compile \
    --fqbn "esp8266:esp8266:generic:xtal=80,vt=iram,exception=disabled,stacksmash=disabled,ssl=all,mmu=3232,non32xfer=fast,ResetMethod=ck,CrystalFreq=26,FlashFreq=40,FlashMode=dout,eesz=1M128,led=13,sdk=nonosdk_190703,ip=lm2f,dbg=Disabled,lvl=None____,wipe=none,baud=115200" \
    --output-dir . \
    --build-property compiler.cpp.extra_flags="-D_VERSION_FRIENDLY_CLI=sonoff_basic" \
    --libraries ./libraries ESP8266-Light-Switch

