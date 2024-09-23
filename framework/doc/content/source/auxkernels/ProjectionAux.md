# ProjectionAux

!syntax description /AuxKernels/ProjectionAux

The `ProjectionAux` can be used to make a copy of a field variable, for example to lag them in certain numerical schemes.
Many `AuxKernels` can use variables as arguments, without modifying them, and store the
result in a separate variable. The use of a `ProjectionAux` can often be avoided for that reason.

The `ProjectionAux` can also be used to project between different finite element variable families and order.
The [!param](/AuxKernels/ProjectionAux/v) parameter is then the source variable.

The default projection method will attempt to enforce that the two variables have the same value on every quadrature point.
If the shape of the source cannot be reproduced by the target variable finite element family and order,
the modeler is invited to measure the projection error using a [ElementL2Difference.md] postprocessor.

!alert note
Elemental variables that are discontinuous at nodes are projected to nodal variables by computing nodal values as element-volume-weighted
averages of the centroid values of neighbor elements.

!alert note
The block restriction of the auxkernel, specified using the [!param](/AuxKernels/ProjectionAux/block) parameter, is used to select the
source variable value as well using the [!param](/AuxKernels/ProjectionAux/use_block_restriction_for_source) parameter.

!alert note
Lower dimensional elements are currently not supported. If you require using lower dimensional elements for projections,
please reach out on the [MOOSE Discussions forum](https://github.com/idaholab/moose/discussions).

!syntax parameters /AuxKernels/ProjectionAux

!syntax inputs /AuxKernels/ProjectionAux

!syntax children /AuxKernels/ProjectionAux
