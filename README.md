# PiThermo
RaspberryPi Thermometer with Colorgauge and Chart

comp: gcc -Wno-unknown-pragmas -Wall -Wextra -o Main PiThermo.c sensor.c dbAccess.c MCP3008.c chart.c $(pkg-config gtk+-3.0 --cflags --libs) -lm -lbcm2835 -lsqlite3 -rdynamic
