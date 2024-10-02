# AnisotropicRankTwoTensor

!syntax description /Materials/AnisotropicRankTwoTensor

The AnisotropicRankTwoTensor generates a rank two tensor material property based on other material properties. This allows the creation of an anisotropic material property tensor, where the values depend on field variables. 

For example: It can be used to define a thermal conductivity tensor as below where elements can be dependent on temperature. 

\[
K = \begin{pmatrix}
K_{xx} & K_{xy} & K_{xz} \\
K_{yx} & K_{yy} & K_{yz} \\
K_{zx} & K_{zy} & K_{zz}
\end{pmatrix}
\]

Here, $K_{xx}, K_{yy}, K_{zz}$ are the thermal conductivities along the $x$, $y$, and $z$ directions, respectively. 

The basis vectors in the Cartesian coordinates, $e_x$, $e_y$, and $e_z$ can be defined for thermal conductivity in $x$, $y$, and $z$ directions. The default basis vectors are $(1,0,0)$, $(0,1,0)$ and $(0,0,1)$.

!syntax parameters /Materials/AnisotropicRankTwoTensor

!syntax inputs /Materials/AnisotropicRankTwoTensor