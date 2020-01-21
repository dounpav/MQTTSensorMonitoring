# MQTTSensorMonitoring

Linux command line interface program for monitoring different sensors using MQTT protocol.

Program works on Master/Slave model. Parent process "shell" creates and manages child processes "shell clients" that subscribe to MQTT topics. Clients read the data coming from MQTT topics and send it to the parent process. MQTT topics are created by "sensor client" programs that create and upload data to the MQTT server.
