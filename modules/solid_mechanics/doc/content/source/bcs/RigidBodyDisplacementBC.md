# RigidBodyDisplacementBC

!syntax description /BCs/RigidBodyDisplacementBC

## Description

`RigidBodyDisplacementBC` prescribes one component of the displacement of a body or a surface
that moves rigidly with a finite translation and rotation. The displacement of a node with
reference coordinates $\boldsymbol{X}$ is

\begin{equation}
\boldsymbol{u} = \boldsymbol{t} + \left(\boldsymbol{R}(\boldsymbol{\theta}) - \boldsymbol{I}\right)
\left(\boldsymbol{X} - \boldsymbol{X}_{ref}\right),
\end{equation}

where $\boldsymbol{X}_{ref}$ is the [!param](/BCs/RigidBodyDisplacementBC/reference_point),
$\boldsymbol{t}$ is the translation of the reference point, and $\boldsymbol{\theta}$ is the
rotation vector, whose direction is the rotation axis and whose magnitude
$\phi = \lVert\boldsymbol{\theta}\rVert$ is the rotation angle in radians. The rotation tensor is
given by the Rodrigues formula

\begin{equation}
\boldsymbol{R}(\boldsymbol{\theta}) = \boldsymbol{I} + \frac{\sin\phi}{\phi}\boldsymbol{K}
+ \frac{1 - \cos\phi}{\phi^2}\boldsymbol{K}^2,
\end{equation}

where $\boldsymbol{K}$ is the skew-symmetric tensor with $\boldsymbol{K}\boldsymbol{a} =
\boldsymbol{\theta}\times\boldsymbol{a}$. The rotation is exact for any angle.

The components of $\boldsymbol{t}$ and $\boldsymbol{\theta}$ are functions of time, given by
[!param](/BCs/RigidBodyDisplacementBC/translations) (one function per spatial dimension) and
[!param](/BCs/RigidBodyDisplacementBC/rotations) (three functions in 3D, or a single function
giving the rotation about the $z$ axis in 2D). Omitting either parameter sets the corresponding
motion to zero. A rigid motion is prescribed by applying one `RigidBodyDisplacementBC` to each
displacement variable with identical motion parameters.

To move an entire meshed body rigidly, apply the boundary condition to a nodeset containing all of
its nodes, generated for example by [ParsedGenerateNodeset.md]. The body may also be a surface
mesh embedded in three dimensions.

The boundary condition can also be used in axisymmetric (RZ) problems, where the components refer
to the radial and axial displacements. A translation along the axis of symmetry is then the only
rigid motion of the revolved body, so only the axial translation function should be nonzero and
the rotation should be omitted.

## Example Input File Syntax

!listing modules/solid_mechanics/test/tests/rigid_body/rigid_motion.i block=BCs/rigid_x

!syntax parameters /BCs/RigidBodyDisplacementBC

!syntax inputs /BCs/RigidBodyDisplacementBC

!syntax children /BCs/RigidBodyDisplacementBC
