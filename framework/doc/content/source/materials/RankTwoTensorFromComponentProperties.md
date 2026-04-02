# RankTwoTensorFromComponentProperties

!syntax description /Materials/RankTwoTensorFromComponentProperties

## Overview

`RankTwoTensorFromComponentProperties` assembles a `RankTwoTensor` material property from scalar 
material properties or numeric constants. This material enables creation of anisotropic 
property tensors where individual components are defined separately.

For example, this can create an anisotropic diffusivity tensor:
\begin{equation}
\mathbf{D}(u) = 
\begin{bmatrix}
D_{11}(u) & D_{12}(u)      & D_{13}(u)      \\
D_{21}(u) & D_{22}(u)      & D_{23}(u)      \\
D_{31}(u) & D_{32}(u)      & D_{33}(u)
\end{bmatrix}
\end{equation}
where $D_{nn}$ can be scalar material property or constant.


## Usage

The material requires exactly 9 tensor components specified in row-major order. Each component
can be:

- A material property name (must be of type `Real`)
- A numeric constant (e.g., `0.0`, `1.5`)

## Example: Diagonal Anisotropic Diffusivity
The following example demonstrates assembling a diagonal diffusivity tensor from two
variable-dependent properties and one constant value:

!listing test/tests/materials/rank_two_tensor_from_component_properties/basic.i
block=Materials
```

This produces the following tensor:

\begin{equation}
\mathbf{D} = \begin{bmatrix}
k_x(T) & 0 & 0.0 \\
0 & k_y(T) & 0 \\
0 & 0 & 1.0
\end{bmatrix}
\end{equation}


!syntax parameters /Materials/RankTwoTensorFromComponentProperties

!syntax inputs /Materials/RankTwoTensorFromComponentProperties

!syntax children /Materials/RankTwoTensorFromComponentProperties