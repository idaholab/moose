# CZMComputeDisplacementJumpTotalLagrangian

!syntax description /Materials/CZMComputeDisplacementJumpTotalLagrangian

## Overview

The `CZMComputeDisplacementJumpTotalLagrangian` calculate the incremental displacement jump across a cohesive zone in the interface natural coordinate system. This object assumes finite strains and does account for the interface rotation caused by deformations or rigid body motion.
The displacement jump in global coordinates is defined as:
\begin{equation}
\llbracket u \rrbracket = u^+ - u^-
\end{equation}
and is related to the the displacement jump in interface coordinates, $\llbracket \hat{u} \rrbracket$ by the the following expression:
\begin{equation}
\llbracket \hat{u}  \rrbracket = Q^T_0 R^T \llbracket u \rrbracket
\end{equation}
In the above expression $Q_0$ is the rotation matrix transforming from global to local coordinates in the initial configuration, and $R$ is rotation matrix associated to the interface midplane deformation gradient, $\hat{F}$.
The interface displacement jump increment is computed as the difference between the current and the old values:
\begin{equation}
 \Delta \llbracket \hat{u} \rrbracket = Q^T_0 \left( \Delta R^{T} \llbracket u \rrbracket + R^{T} \Delta\llbracket u \rrbracket  \right)
\end{equation}
The rotation matrix $\Delta R$ represents the incremental rotation between two steps and is computed as:
\begin{equation}
\Delta R = R - R_{old}
\end{equation}
This object also computes the total displacement jump in local coordinates  as:
\begin{equation}
 \llbracket \hat{u} \rrbracket = \Delta \llbracket \hat{u} \rrbracket + \llbracket \hat{u} \rrbracket_{old}
\end{equation}
The first component of $\llbracket \hat{u} \rrbracket$ always refers to the displacement jump in the normal direction, the other two components refers to tangential directions.
This object can be used in 1D 2D and 3D simulation.

## Example Input File Syntax

This object is automatically added from the [Cohesive Master Master Action](CohesiveZoneMaster/index.md) when `kinematic=TotalLagrangian`.

!syntax parameters /Materials/CZMComputeDisplacementJumpTotalLagrangian

!syntax inputs /Materials/CZMComputeDisplacementJumpTotalLagrangian

!syntax children /Materials/CZMComputeDisplacementJumpTotalLagrangian
