# Cylindrical Rank Two Aux

!syntax description /AuxKernels/CylindricalRankTwoAux

## Description

The AuxKernel `CylindricalRankTwoAux` transforms a Rank-2 tensor, $T$, into cylinderical coordinates,
where the cylindrical rotation axis is along the Cartesian $\hat{z}$ axis and the user-defined center
point lies within the Cartesian $\hat{x}$-$\hat{y}$ plane, as shown in [eq:cylindrical_rank_two_aux].
The AuxKernel will save the component of the tranformed Rank-2 tensor, $T^R$, as defined by the
arguments for the `index_i` and `index_j` parameters.
\begin{equation}
\label{eq:cylindrical_rank_two_aux}
T^R_{ij} = \boldsymbol{R} \cdot \boldsymbol{T} \cdot \boldsymbol{R}^T
\end{equation}
The rotation tensor $R$ is defined as
\begin{equation}
\label{eq:cylindrical_rotation_tensor}
  R = \begin{bmatrix}
      cos(\theta) & sin(\theta) \\
      -sin(\theta) & cos(\theta)
      \end{bmatrix}
      \quad \text{ where } \quad \theta = atan2 \left( \frac{P^{qp}_y - P^c_y}{P^{qp}_x - P^c_x} \right)
\end{equation}
where $P^{qp}$ is the location of the current quadrature point being evaluated and $P^c$ is the
center point defined by the user with the parameter `center_point` in the input file.

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/CylindricalRankTwoAux/test.i block=AuxKernels/stress_tt

and an AuxVariable is required to store the AuxKernel information. Note that the name of the
AuxVariable is used as the argument for the `variable` input parameter in the `CylindricalRankTwoAux`
block.

!listing modules/tensor_mechanics/test/tests/CylindricalRankTwoAux/test.i block=AuxVariables/stress_tt

!syntax parameters /AuxKernels/CylindricalRankTwoAux

!syntax inputs /AuxKernels/CylindricalRankTwoAux

!syntax children /AuxKernels/CylindricalRankTwoAux
