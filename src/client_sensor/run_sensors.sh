
#!bin/bash

./sensor_client ./sensors/sensor_simulator 127.0.0.1 room1/temperature &
./sensor_client ./sensors/sensor_simulator 127.0.0.1 room1/humidity &
./sensor_client ./sensors/sensor_simulator 127.0.0.1 room2/temperature &
