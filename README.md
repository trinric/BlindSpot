# BlindSpot
 An overengineered window blind adjuster for the arduino.

The goal of this was to have my blinds automatically adjust based on the current angle of the sun so I can get the optimal amount of sunlight, but also keep it out of my eyes. The calculations are completed using only latitude / longitude and current time, and should be valid for the next roughly hundred years (within a degree or 2 of accuracy). Most of the length of the code is just doing the necessary calculations.

You will need to adjust many of the parameters before uploading to an arduino, such as eye height, distance from the wall, etc. 

The motion calculations were done using a standard pully/string window blind, and a stepper motor attached to a spool that locks into place.  
