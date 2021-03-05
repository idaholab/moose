# TwistingMomentBC

!syntax description /BCs/TwistingMomentBC

## Overview

This boundary condition applies a torque $T$ (`direction`) with respect to a reference
point (`origin`) to a surface. It requires the calculation of the *polar moment of inertia*
$J$ (or $I_z$), which can be performed using the [PolarMomentOfInertia](PolarMomentOfInertia.md)
postprocessor.

The application of a twisting moment is currently only validated for small strains.

## Example Input File Syntax

!syntax parameters /BCs/TwistingMomentBC

!syntax inputs /BCs/TwistingMomentBC

!syntax children /BCs/TwistingMomentBC
