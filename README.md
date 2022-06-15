# PiThermo
RaspberryPi Thermometer with Colorgauge and Chart

comp: gcc -Wno-unknown-pragmas -Wall -Wextra -o Main PiThermo.c sensor.c dbAccess.c MCP3008.c chart.c $(pkg-config gtk+-3.0 --cflags --libs) -lm -lbcm2835 -lsqlite3 -rdynamic
![Pi-Thermo](https://user-images.githubusercontent.com/88623443/173812002-424be49d-b18d-4c10-a8dc-bde526c6c234.png)
