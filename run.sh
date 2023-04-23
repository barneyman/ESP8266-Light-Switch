#!/bin/sh
docker build -f ./.devcontainer/Dockerfile --target interactive --tag esp8266builder .

docker run --rm -it --device=/dev/ttyUSB0 -v $PWD:/workspaces/ESP8266-Light-Switch esp8266builder /bin/bash