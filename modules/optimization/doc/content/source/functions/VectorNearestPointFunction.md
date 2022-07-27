# VectorNearestPointFunction

!syntax description /Functions/VectorNearestPointFunction

## Overview

This function computes values based on coordinate and time data. This data comes in the form of vector reporter or vectorpostprocessor values. The required parameter [!param](/Functions/VectorNearestPointFunction/value) is the name of the vector containing the values used to set the variable to. [!param](/Functions/VectorNearestPointFunction/coord_x)/[!param](/Functions/VectorNearestPointFunction/coord_y)/[!param](/Functions/VectorNearestPointFunction/coord_z) are vectors with the x-/y-/z-coordinate data. The auxkernel will set the variable's nodal/elemental values based on the nearest point in this data. [!param](/Functions/VectorNearestPointFunction/time) is the name of the time data; the kernel will linearly interpolate between two closest times for a certain coordinate, without extrapolating. [!param](/Functions/VectorNearestPointFunction/coord_x), [!param](/Functions/VectorNearestPointFunction/coord_y), [!param](/Functions/VectorNearestPointFunction/coord_z), and [!param](/Functions/VectorNearestPointFunction/time) will assume to be 0 when not specified. When specified, [!param](/Functions/VectorNearestPointFunction/coord_x), [!param](/Functions/VectorNearestPointFunction/coord_y), [!param](/Functions/VectorNearestPointFunction/coord_z), and [!param](/Functions/VectorNearestPointFunction/time) must all be the same length as [!param](/Functions/VectorNearestPointFunction/value).

## Example Input File Syntax

Here are several examples different combinations of x, y, z, and time data is specified:

!listing vector_nearest_point.i block=Functions Reporters

Run this test to see how the function interpolates the data between specified times.

!syntax parameters /Functions/VectorNearestPointFunction

!syntax inputs /Functions/VectorNearestPointFunction

!syntax children /Functions/VectorNearestPointFunction
