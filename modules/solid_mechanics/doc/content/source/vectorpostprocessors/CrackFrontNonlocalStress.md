# CrackFrontNonlocalStress

!syntax description /VectorPostprocessors/CrackFrontNonlocalStress

## Description

This object computes the average scalar stress normal to the crack extension direction defined by the [CrackFrontDefinition.md] given by the equation:
\begin{equation}
\sigma_{nn} = \boldsymbol{n} \cdot\boldsymbol{\sigma}\cdot \boldsymbol{n}
\end{equation}

where $\boldsymbol{n}$ is the normal direction vector and $\boldsymbol{\sigma}$ is the stress tensor.

Data produced by this VectorPostprocessor is used in conjunction with the [InteractionIntegral.md] in the XFEM module by the `MeshCut2DFractureUserObject` to grow cracks. The `CrackFrontNonlocalStress` is useful for extending cracks that are approaching free surfaces because the interaction integrals computing `KI` and `KII` are affected when the integration domain intersects the free surface.  `CrackFrontNonlocalStress` computes an average of the stress over a box-shaped domain at each crack tip point that is centered on the crack tip and extends [!param](/VectorPostprocessors/CrackFrontNonlocalStress/box_length) in front of the crack tip.  The [!param](/VectorPostprocessors/CrackFrontNonlocalStress/box_height) is the dimension normal to the crack face, and [!param](/VectorPostprocessors/CrackFrontNonlocalStress/box_width) is the dimension tangential to the crack face.  [!param](/VectorPostprocessors/CrackFrontNonlocalStress/box_width) is not used in 2D problems.  Unlike the other stress integrals, like the [InteractionIntegral.md], that use the [CrackFrontDefinition.md], `CrackFrontNonlocalStress` is not set-up by the [/DomainIntegralAction.md].

In the following input file example, the mesh consists of a 3D plate with a hole in the middle. The CrackFrontDefinition defines crack points around the center line of the hole, `boundary=1001`. The `CrackFrontNonlocalStress` averages a generic stress field over the box-shaped region at each crack front point, with dimensions defined in the input file.

!listing crack_front_nonlocal_materials.i block=UserObjects VectorPostprocessors/CrackFrontNonlocalStress

!syntax parameters /VectorPostprocessors/CrackFrontNonlocalStress

!syntax inputs /VectorPostprocessors/CrackFrontNonlocalStress

!syntax children /VectorPostprocessors/CrackFrontNonlocalStress
