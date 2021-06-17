# Torque

!syntax description /BCs/Torque

## Overview

This boundary condition applies a torque $T$ (`direction`) with respect to a
reference point (`origin`) through tractions distributed over a surface
according to the right hand rule. It requires the calculation of the *polar
moment of inertia* $J$ (or $I_z$), which can be performed using the
[PolarMomentOfInertia](PolarMomentOfInertia.md) postprocessor.

The application of a torque is currently only validated for small strains.

## Example Input File Syntax

!syntax parameters /BCs/Torque

!syntax inputs /BCs/Torque

!syntax children /BCs/Torque
