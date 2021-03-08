# DeflectionAngle

!syntax description /AuxKernels/DeflectionAngle

## Overview

This AuxKernel computes the deflection angle $\alpha$ in radians for every node from the
given `displacements` variables. The angle is calculated between a vector $\vec r_1$ from a
given reference point (the `origin`) to the undisplced node and a vector $\vec r_2$
from that point to the displaced node.

If a reference `direction` $\vec d$ if provided the angle is measured in the plane
defined by the direction vector. This is achieved by normalizing the direction
and subtracting the projection of $\vec r_{1,2}$ onto $\vec d$ from $\vec r_{1,2}$.

The angle $\alpha$ is computed as

!equation
\alpha = \acos\frac{\vec r_1 \cdot \vec r_1}{|r_1|\cdot|r_2|}

If the denominator of this fraction is too small, zero is returned as the angle.

## Example Input File Syntax

!syntax parameters /AuxKernels/DeflectionAngle

!syntax inputs /AuxKernels/DeflectionAngle

!syntax children /AuxKernels/DeflectionAngle
