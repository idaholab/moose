# EBSDReader

!syntax description /UserObjects/EBSDReader

The EBSDReader user object reads in data from [electron backscatter diffraction (EBSD)](https://en.wikipedia.org/wiki/Electron_backscatter_diffraction)
to set initial conditions for phase field simulations. EBSD identifies the grain and
phase structure of a sample, as well as the crystal structure and the crystal orientation.
The `EBSDReader` reads in this information from a specially formatted data file and
reconstructs the microstructure. It accomplishes this by setting the initial conditions
for all the phase field variables. In addition, it stores the crystal structure and
orientation information by use by the phase field and mechanics models.

**Note that the EBSDReader does not mesh the microstructure**, but rather sets phase
field variable initial conditions. To reconstruct a mesh of the microstructure,
we recommend using [OOF](http://www.ctcms.nist.gov/oof/).

### Average orientation of a grain

Euler angles are not the best for representing the orientation of a grain because they suffer from gimbal lock. Therefore, we use quaternions to compute the average orientation of the grain. Firstly, the Euler angles are converted to quaternions. Next, we segregate the quaternions into different bins with the parameter "bins" specifying the number of bins. We then perform a weighted average of the quaternions with weights of each quaternion corresponds
to size of the bin in which it resides. The average quaternion is then converted to Euler angle representation.

Various weighted averages can be performed by raising power of the weights corresponding to each quaternion. The parameter "L_norm" allows us to
raise the power of the weights according to the positive integer assigned to it. By default, the value of "L_norm" is set to 1, computing the simple weighted average of the quaternions.

!syntax parameters /UserObjects/EBSDReader

# EBSD Data File Format

The `EBSDReader` user object reads in a data file taken from EBSD data. The file
should have the following format:

```
# Header:    Marmot Input File

# Date:      19-Jul-2013 00:23:55

#
# Column 1:  Euler angle "phi1" (in radians)

# Column 2:  Euler angle "PHI" (in radians)

# Column 3:  Euler angle "phi2" (in radians)

# Column 4:  x-coordinate (in microns)

# Column 5:  y-coordinate (in microns)

# Column 6:  z-coordinate (in microns)

# Column 7:  grain number (integer)

# Column 8:  phase number (integer)

# Column 9:  symmetry class (from TSL)

#
# Phase 1:   Nickel (symmetry class = 43)

# Number of Grains in Phase 1:  111

#
# X_Min:      0.000000

# X_Max:      32.000000

# X_step:     0.250000

# X_Dim:      128

#
# Y_Min:      0.000000

# Y_Max:      32.000000

# Y_step:     0.250000

# Y_Dim:      128

#
# Z_Min:      0.000000

# Z_Max:      0.000000

# Z_step:     0.000000

# Z_Dim:      0

#
2.48663 1.84098 5.50548 0.12500 0.12500 0.00000 0 1 43
2.48663 1.84098 5.50548 0.12500 0.37500 0.00000 0 1 43
.
.
.
```

The open source code [Dream3D](http://dream3d.bluequartz.net/) has the option to
output in this format using the _"Write INL File"_ filter. Note that the data must
be on a square grid rather than a hex grid.

The `EBSDReader` supports additional custom data columns.

!syntax inputs /UserObjects/EBSDReader

!syntax children /UserObjects/EBSDReader
