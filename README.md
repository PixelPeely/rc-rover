# rc-rover
Arduino-powered rc rover with live video feed

# Controller
## Custom circuit board:
The Controller PCB is responsible for charging a LiCD battery, as well as powering the screen board, which is an independent unit. The board contains through holes for the NRF24L01+ module, along with 4 LMS-5161 common anode 7 segment displays that are controlled by 4 74HC595 8-bit shift registers, all of which are controlled by an arduino nano every and powered by the LiCD battery.
Schematic:
![alt text](https://github.com/PixelPeely/rc-rover/blob/main/Controller/PCB/Schematic.png?raw=true)
Footprint:
![alt text](https://github.com/PixelPeely/rc-rover/blob/main/Controller/PCB/Footprint.png?raw=true)
3D Render:
![alt text](https://github.com/PixelPeely/rc-rover/blob/main/Controller/PCB/3D_Render.png?raw=true)
