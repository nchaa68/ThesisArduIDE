# MedStance Device 

This is code for the device of MedStance. It uses 2 ESP32 that are connected to WiFii and Firebase. 
Consist of slave and Master for handling different function 

# Master
This is the code that are functioning for sending data and recieving data of timer (medicine consumption time) and medicine (quantity,location, and names) from firebase. 
The data in the firebase are saved from the user using website as the interface. Buzzer will on whenever the time  in the firebase matched the real time (verified with RTC)

This master sending information and instruction to the slaves, so the slaves can send instruction to the mechanical device in order to dispense the medicine from the container.

# Slave
this is the code that are functioning for recieving data using serial communication UART with the master and excuting the action to dispense the corresponding medicine. 
After dispensing the medicine, this ESP32 slave are sending data to update data in the firebase. 
