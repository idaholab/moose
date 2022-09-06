# MortarPressureComponentAux

## Overview

This object transforms the Lagrange multiplier (LM) variable from the Cartesian coordinate system to the local coordinate system and outputs each individual component along the normal or the tangential direction.

The definition of LM variable on node $j$ in the Cartesian coordinate system is
\begin{equation}
\boldsymbol{z}_j = (z_x)_j (\boldsymbol{v}_x)_j + (z_y)_j (\boldsymbol{v}_y)_j + (z_z)_j (\boldsymbol{v}_z)_j,
\end{equation}
where $z_x$, $z_y$, $z_z$ are the LM variable components along the $x$, $y$, $z$ directions, respectively. Here, $z_z$ vanishes for two-dimensional problems. The $\boldsymbol{v}_x$ $\boldsymbol{v}_y$, $\boldsymbol{v}_z$ are the normal vectors along the three directions.

In the local coordinate system, the definition of LM variable is
\begin{equation}
\boldsymbol{z}_j = (z_n)_j \boldsymbol{n}_j + (z_\tau^{1})_j \boldsymbol{\tau}^{1}_j + (z_\tau^{2})_j \boldsymbol{\tau}^{2}_j ,
\end{equation}
where $z_n$ is the component that is normal to the contact surface,  $\boldsymbol{n}$ is the local normal vector. For frictional problems in two dimnsions, the $z_\tau^{1}$ is the tangential component that is aligned with the contact surface, $\boldsymbol{\tau}^{1}$ is the corresponding tangential vector. In three-dimensional frictional problems, an additional component, $z_\tau^{2}$ exist along the contact surface, which is associated with the second tangential vector $\boldsymbol{\tau}^{2}$.


!syntax parameters /AuxKernels/MortarPressureComponentAux

!syntax inputs /AuxKernels/MortarPressureComponentAux

!syntax children /AuxKernels/MortarPressureComponentAux
