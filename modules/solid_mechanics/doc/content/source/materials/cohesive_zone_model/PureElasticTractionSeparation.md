# PureElasticTractionSeparation

!syntax description /Materials/PureElasticTractionSeparation

## Description

This material implements a pure elastic traction separation law defined by
\begin{equation}
T =K \llbracket u \rrbracket
\end{equation}
where $K$ is a diagonal stiffness matrix. The user only need to define the `normal` and `tangent` stiffness.
This model can be used for 1D, 2D and 3D problems.

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/cohesive_zone_model/stretch_rotate_large_deformation.i block=Materials/czm_mat

!syntax parameters /Materials/PureElasticTractionSeparation
!syntax inputs /Materials/PureElasticTractionSeparation
!syntax children /Materials/PureElasticTractionSeparation
