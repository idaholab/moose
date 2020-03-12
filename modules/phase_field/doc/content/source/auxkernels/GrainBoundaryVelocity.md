# GrainBoundaryVelocity

This will compute the velocity of the grain boundary edge and display those values for
visualization purposes. Only one AuxVariable is required, and optional variables for
the range we are interested in are also available.

## Overview

we are using the equation:
\begin{equation}
\frac{1}{\left | \triangledown \eta \right |} \frac{d\eta}{dt} = v
\end{equation}
which computes the perpendicular velocity at each quadrature point in the range
specified.

## Example Input File Syntax

!listing modules/phase_field/test/tests/Grain_Velocity_Computation/GrainBoundaryVelocityTest.i block=AuxVariables/velocity

!listing modules/phase_field/test/tests/Grain_Velocity_Computation/GrainBoundaryVelocityTest.i block=AuxKernels/velocity

!syntax parameters /AuxKernels/GrainBoundaryVelocity

!syntax inputs /AuxKernels/GrainBoundaryVelocity

!syntax children /AuxKernels/GrainBoundaryVelocity
