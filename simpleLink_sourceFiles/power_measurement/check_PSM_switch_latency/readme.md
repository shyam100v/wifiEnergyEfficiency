This file should replace the power_measure.c fource file inside the power_measurement project example. 

This configures the device to send 1 TCP packet every second. The device is woken up from PSM befpre sending the packet and PSM is enabled with a timer. The purpose of the project was to check how long this switching takes. 

Configure the following as necessary: TCP server IP and port numbers, TCP segment size, traffic frequency.