# CrackFrontNormalStress

!syntax description /VectorPostprocessors/CrackFrontNormalStress

## Description

This object computes the average scalar stress normal to the crack face defined by the [CrackFrontDefinition.md] given by the equation:
\begin{equation}
\sigma_{nn} = \boldsymbol{n} \cdot\boldsymbol{\sigma}\cdot \boldsymbol{n}
\end{equation}

where $\boldsymbol{n}$ is the normal vector to the crack face and $\boldsymbol{\sigma}$ is the stress tensor.

Data produced by this vectorPostProcessor is used in conjunction with the [InteractionIntegral.md] in the XFEM module by the `MeshCut2DFractureUserObject` to grow cracks. The `CrackFrontNormalStress` is useful for extending cracks near free surfaces where the interaction integrals computing `KI` and `KII` are reduced when the integration domain intersects the free surface.  The volume being averaged over by `CrackFrontNormalStress` is a box at each crack tip point that is centered on the crack tip and extends [!param](/VectorPostprocessors/CrackFrontNormalStress/box_length) in front of the crack tip.  The [!param](/VectorPostprocessors/CrackFrontNormalStress/box_height) is the dimension normal to the crack face, and [!param](/VectorPostprocessors/CrackFrontNormalStress/box_width) is the dimension tangential to the crack face.  [!param](/VectorPostprocessors/CrackFrontNormalStress/box_width) is not used in 2D problems.  Unlike the other stress integrals, like the [InteractionIntegral.md], that use the [CrackFrontDefinition.md], `CrackFrontNormalStress` is not set-up by the [/DomainIntegralAction.md].

In the following input file example, the mesh consists of a 3D plate with a hole in the middle. The CrackFrontDefinition defines crack points around the center line of the hole, `boundary=1001`. This `CrackFrontNormalStress` integrates a generic stress field set-up in the input file over the box dimensions shown.

!listing crack_front_normal_stress.i block=UserObjects VectorPostprocessors

!syntax parameters /VectorPostprocessors/CrackFrontNormalStress

!syntax inputs /VectorPostprocessors/CrackFrontNormalStress

!syntax children /VectorPostprocessors/CrackFrontNormalStress
