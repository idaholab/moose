# Material Tensor Integral

!syntax description /Postprocessors/MaterialTensorIntegral

## Description

The `MaterialTensorIntegral` postprocessor computes the volume integral of the
Rank-2 tensor component specified by the user.
\begin{equation}
  \label{eqn:volume_integal_tensor_comp}
  S^{int} = \int_V T_{ij} dV
\end{equation}
where $S^{int}$ is the computed volume integral quantity and $T_{ij}$ is the
tensor component selected by the user.
The tensor component indicies, $i$ and $j$, range from 0 to 2 as shown in the
reference tensor
\begin{equation}
  \label{eqn:ref_tensor}
  T_{ij} = \begin{bmatrix}
            0,0 & 0,1 & 0,2 \\
            1,0 & 1,1 & 1,2 \\
            2,0 & 2,1 & 2,2
            \end{bmatrix}
\end{equation}

This class is most often used in
[Generalized Plane Strain](modules/tensor_mechanics/generalized_plane_strain.md)
simulations to calculate the out-of-plane stress component.

## Example Input File

!listing modules/tensor_mechanics/test/tests/generalized_plane_strain/plane_strain.i block=Postprocessors

!syntax parameters /Postprocessors/MaterialTensorIntegral

!syntax inputs /Postprocessors/MaterialTensorIntegral

!syntax children /Postprocessors/MaterialTensorIntegral
