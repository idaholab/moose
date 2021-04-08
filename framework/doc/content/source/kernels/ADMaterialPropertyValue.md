# ADMaterialPropertyValue

!syntax description /Kernels/ADMaterialPropertyValue

This kernel sets the kernel variable $u$ (`variable`) weekly enforced to the
value of a material  property $p$ (`prop`).

!equation
\left(\pm(p-u),\psi\right)

The $\pm$ sign is controlled by the `positive` boolean parameter.

This kernel can be used to emulate the action of nodal patch recovery, by finding
a projection of an interior material property onto (nodal) basis function degrees
of freedom. It can be used to generate a smooth field for outputting material
properties.

!syntax parameters /Kernels/ADMaterialPropertyValue

!syntax inputs /Kernels/ADMaterialPropertyValue

!syntax children /Kernels/ADMaterialPropertyValue

!bibtex bibliography
