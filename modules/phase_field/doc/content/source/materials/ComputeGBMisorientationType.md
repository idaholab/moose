# ComputeGBMisorientationType

ComputeGBMisorientationType uses an EBSDReader object and a Grain Tracker object to calculate the grain boundary (GB) type based on the misorientation angle. There are two kind of GB type: Low Angle GB (LAGB) and High Angle GB (HAGB). When the misorientation angle is smaller than the critical angle, 15˚, the GB type is defined as LAGB, and it is defined as HAGB for higher misorientation angles. The critical angle can be modified with the [!param](/Materials/ComputeGBMisorientationType/angle_threshold) parameter. 

## Description and Syntax

!syntax description /Materials/ComputeGBMisorientationType

!syntax parameters /Materials/ComputeGBMisorientationType

!syntax inputs /Materials/ComputeGBMisorientationType