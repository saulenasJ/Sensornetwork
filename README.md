# Sensornetwork
Module A sends data to module B, then receives a byte to Characteristic_Sleep and goes to sleep. Module B, after turning off module A, creates server and sends data to module C. Module C puts module B to sleep and uploads data to MQTT broker, then goes to sleep himself. 

When back from sleep, module B does not connect to module A again. 
