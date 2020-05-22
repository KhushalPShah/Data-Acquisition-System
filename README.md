# Data-Acquisition-System
An Embedded Data Acquisition System using LPC1768 and Raspberry Pi 3.

# The project consists of 2 Parts - 
1. Client (LPC1768)
2. Server (Raspberry Pi 3)

# Workflow:
The LPC1768 generates 2 dummy values - 
1. An analog sensor value (From 0 to 4096)
2. Digital Sensor state (0 or 1)

This is then combined into a Packet, and sent to the Server over a UDP-IP Protocol, with the Ethernet as the Physical layer.

The Raspberry Pi, which runs a Node-red server with an open UDP Port, accepts this UDP Packet, extracts the data from the packet 
and updates the Node-red Dashboard Webpage.

# Technology Stack:
For Client : 
LwIP for enabling the Networking on the LPC1768.

For Server:
Stretch Lite OS running on Raspberry Pi 3, with a Node-red Server.

# Softwares:
- Eclipse for LPC1768 related coding.
- Any Browser for server side coding on Nodered.
#
