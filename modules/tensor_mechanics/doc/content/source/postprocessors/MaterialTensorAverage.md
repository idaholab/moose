# Material Tensor Average

!syntax description /Postprocessors/MaterialTensorAverage

## Description

The `MaterialTensorAverage` postprocessor computes the volume average of the
Rank-2 tensor component specified by the user.
\begin{equation}
  \label{eqn:volume_integal_tensor_comp}
  S^{avg} = \int_V T_{ij} dV / \int_V dv
\end{equation}
where $S^{avg}$ is the computed volume average quantity and $T_{ij}$ is the
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

The parameter `use_displaced_mesh` controls the volume utilized to compute the average. If `use_displaced_mesh=true` the average is compute utilizing the deformed volume, if `use_displaced_mesh=false` (default) the average is compute utilizing the initial volume.

## Example Input File

!listing modules/tensor_mechanics/test/tests/postprocessors/material_tensor_average_test.i block=Postprocessors

!syntax parameters /Postprocessors/MaterialTensorAverage

!syntax inputs /Postprocessors/MaterialTensorAverage

!syntax children /Postprocessors/MaterialTensorAverage
