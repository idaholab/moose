# CZMComputeDisplacementJumpSmallStrain

!syntax description /Materials/CZMComputeDisplacementJumpSmallStrain

## Overview

The `CZMComputeDisplacementJumpSmallStrain` calculate the total displacement jump across a cohesive zone in the interface natural coordinate system. This object assumes small strain and does not account for the interface rotation caused by deformations or rigid body motion.
The displacement jump in global coordinates is defined as:
\begin{equation}
\llbracket u \rrbracket = u^+ - u^-
\end{equation}
and is related to the the displacement jump in interface coordinates, \llbracket \hat{u} \rrbracket by the the following expression:
\begin{equation}
\llbracket \hat{u}  \rrbracket = Q^T_0 \llbracket u \rrbracket
\end{equation}
In the above expression $Q^0$ is the rotation matrix transforming from global to local coordinates in the initial configuration.
The first component of \llbracket \hat{u} \rrbracket always refers to the displacement jump in the normal direction, the other two components refers to tangential directions.
This object can be used in 1D 2D and 3D simulation

## Example Input File Syntax

This object is automatically added from the [Cohesive Master Master Action](CohesiveZoneMaster/index.md) when `strain=SMALL`.

!syntax parameters /Materials/CZMComputeDisplacementJumpSmallStrain

!syntax inputs /Materials/CZMComputeDisplacementJumpSmallStrain

!syntax children /Materials/CZMComputeDisplacementJumpSmallStrain
