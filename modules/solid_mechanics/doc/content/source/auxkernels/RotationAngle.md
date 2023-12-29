# RotationAngle

!syntax description /AuxKernels/RotationAngle

## Overview

This AuxKernel computes the rotation angle $\alpha$ in radians around an axis
given by a reference point (the `origin`) and a `direction` vector $\vec d$ for
every node. The vectors $\vec r_1$ from the origin to the undisplaced node and
$\vec r_2$ from the to the displaced node are computed. The displaced point is
determined by adding the `displacements` components to the undisplaced node
location. Projection of $\vec r_{1,2}$ onto the normalized direction
$\frac1{|d|}\vec d$ are then subtracted from $\vec r_{1,2}$.

The angle $\alpha$ is computed as

!equation
\alpha = \arccos\frac{\vec r_1 \cdot \vec r_1}{|r_1|\cdot|r_2|} \cdot \text{sign}\left((\vec r_1 \times \vec r_2)\cdot \vec d\right)

If the denominator of this fraction is too small, zero is returned as the angle.
The sign of the angle is determined by comparing the cross product of the
projected $\vec r_{1,2}$ vectors to the direction vector $\vec d$. Right hand
rule applies.

## Example Input File Syntax

!syntax parameters /AuxKernels/RotationAngle

!syntax inputs /AuxKernels/RotationAngle

!syntax children /AuxKernels/RotationAngle
