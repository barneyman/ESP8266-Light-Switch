FROM ubuntu:latest AS devcontainer

#RUN echo 'Acquire::http { Proxy "http://apt-cache:3142"; };' > /etc/apt/apt.conf.d/01proxy && cat /etc/apt/apt.conf.d/01proxy

RUN apt update && apt upgrade -y 

RUN apt install -y curl git make build-essential python3 pip wget libffi-dev

# esp32 builds needs these
RUN ln -s /usr/bin/python3 /usr/bin/python
RUN pip install pyserial --break-system-packages

# keep in sync with the action
RUN curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh -s 0.22.0

RUN arduino-cli core update-index --additional-urls https://arduino.esp8266.com/stable/package_esp8266com_index.json,https://espressif.github.io/arduino-esp32/package_esp32_index.json
RUN arduino-cli core install esp8266:esp8266@3.0.2 --additional-urls https://arduino.esp8266.com/stable/package_esp8266com_index.json 
RUN arduino-cli core install esp32:esp32@2.0.2 --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json --verbose

# WORKDIR /spiffs
# RUN wget https://github.com/igrr/mkspiffs/releases/download/0.2.3/mkspiffs-0.2.3-arduino-esp32-linux64.tar.gz && \
#         tar -xvf mkspiffs-0.2.3-arduino-esp32-linux64.tar.gz --strip-components 1 && \
#         mv mkspiffs mkspiffs_esp32

# RUN wget https://github.com/igrr/mkspiffs/releases/download/0.2.3/mkspiffs-0.2.3-arduino-esp8266-linux64.tar.gz && \
#         tar -xvf mkspiffs-0.2.3-arduino-esp8266-linux64.tar.gz --strip-components 1 && \
#         mv mkspiffs mkspiffs_esp8266

# RUN rm *.gz

RUN pip3 install esptool --break-system-packages

RUN apt install -y udev

WORKDIR /

FROM devcontainer AS interactive

RUN apt install -y screen

VOLUME /workspaces/ESP8266-Light-Switch
WORKDIR /workspaces/ESP8266-Light-Switch
