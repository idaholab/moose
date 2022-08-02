# ComputeGBMisorientationType

ComputeGBMisorientationType uses an EBSDReader object and a Grain Tracker object to calculate the grain boundary (GB) type based on the misorientation angle. There are two kind of GB type: Low Angle GB (LAGB) and High Angle GB (HAGB). When the misorientation angle is smaller than the critical angle, 15˚, the GB type is defined as LAGB, and it is defined as HAGB for higher misorientation angles. The critical angle can be modified with the "angle_threshold" parameter. The misorientation file can be provided by the "file_name" parameter. The row number $n$ of misorientation angle in the file is defined as

\begin{equation}
n = g_i+(g_j-1)*g_j/2
\end{equation}

The $g_i$ is the EBSD id for grain $i$, the $g_j$ is the EBSD id for grain $j$, and grain $i$ should always smaller than grain $j$ to make sure all GBs will appear once.

## Description and Syntax

!syntax description /Materials/ComputeGBMisorientationType

!syntax parameters /Materials/ComputeGBMisorientationType

!syntax inputs /Materials/ComputeGBMisorientationType