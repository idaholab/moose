# NearestReporterCoordinatesFunction

!syntax description /Functions/NearestReporterCoordinatesFunction

## Overview

This function computes values based on coordinate and time data. This data comes in the form of vector reporter or vectorpostprocessor values. The required parameter [!param](/Functions/NearestReporterCoordinatesFunction/value_name) is the name of the vector containing the values used to set the variable to. [!param](/Functions/NearestReporterCoordinatesFunction/x_coord_name)/[!param](/Functions/NearestReporterCoordinatesFunction/y_coord_name)/[!param](/Functions/NearestReporterCoordinatesFunction/z_coord_name) are vectors with the x-/y-/z-coordinate data. The auxkernel will set the variable's nodal/elemental values based on the nearest point in this data. [!param](/Functions/NearestReporterCoordinatesFunction/time_name) is the name of the time data; the kernel will linearly interpolate between two closest times for a certain coordinate, without extrapolating. [!param](/Functions/NearestReporterCoordinatesFunction/x_coord_name), [!param](/Functions/NearestReporterCoordinatesFunction/y_coord_name), [!param](/Functions/NearestReporterCoordinatesFunction/z_coord_name), and [!param](/Functions/NearestReporterCoordinatesFunction/time_name) will assume to be 0 when not specified. When specified, [!param](/Functions/NearestReporterCoordinatesFunction/x_coord_name), [!param](/Functions/NearestReporterCoordinatesFunction/y_coord_name), [!param](/Functions/NearestReporterCoordinatesFunction/z_coord_name), and [!param](/Functions/NearestReporterCoordinatesFunction/time_name) must all be the same length as [!param](/Functions/NearestReporterCoordinatesFunction/value_name).

## Example Input File Syntax

Here are several examples of different combinations where x, y, z, and time data is specified:

!listing nearest_reporter_point.i block=Functions Reporters

Run this test to see how the function interpolates the data between specified times.

!syntax parameters /Functions/NearestReporterCoordinatesFunction

!syntax inputs /Functions/NearestReporterCoordinatesFunction

!syntax children /Functions/NearestReporterCoordinatesFunction
