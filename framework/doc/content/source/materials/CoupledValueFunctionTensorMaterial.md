# CoupledValueFunctionTensorMaterial

!syntax description /Materials/CoupledValueFunctionTensorMaterial

## Overview

`CoupledValueFunctionTensorMaterial` assembles a `RankTwoTensor` material property from scalar 
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

For a diagonal tensor with different diffusivities in each direction:

```
[Materials]
  [D_x_property]
    type = CoupledValueFunctionMaterial
    function = D_x_func
    prop_name = D_x
    v = concentration
  []
  [D_y_property]
    type = CoupledValueFunctionMaterial
    function = D_y_func
    prop_name = D_y
    v = concentration
  []
  [D_z_property]
    type = CoupledValueFunctionMaterial
    function = D_z_func
    prop_name = D_z
    v = concentration
  []
  
  [diffusivity_tensor]
    type = CoupledValueFunctionTensorMaterial
    tensor_name = D
    tensor_values = 'D_x  0.0  1.0
                     0.0  D_y  0.0
                     0.0  0.0  D_z'
  []
[]
```

This produces the following tensor:

\begin{equation}
\mathbf{D} = \begin{bmatrix}
D_x(u) & 0 & 1.0 \\
0 & D_y(u) & 0 \\
0 & 0 & D_z(u)
\end{bmatrix}
\end{equation}


!syntax parameters /Materials/CoupledValueFunctionTensorMaterial

!syntax inputs /Materials/CoupledValueFunctionTensorMaterial

!syntax children /Materials/CoupledValueFunctionTensorMaterial