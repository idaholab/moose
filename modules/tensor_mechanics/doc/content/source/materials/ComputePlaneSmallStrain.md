# Compute Plane Small Strain

!syntax description /Materials/ComputePlaneSmallStrain

## Description

The material `ComputePlaneSmallStrain` calculates the small total
strain for 2D plane strain problems. It can be used for classical
[plane strain or plane stress](https://en.wikipedia.org/wiki/Plane_stress)
problems, or in
[Generalized Plane Strain](tensor_mechanics/generalized_plane_strain.md) simulations.

## Out of Plane Strain

In the classical plane strain problem, it is assumed that the front and back
surfaces of the body are constrained in the out-of-plane direction, and that
the displacements in that direction on those surfaces are zero. As a
result, the strain and deformation gradient components in the out-of-plane
direction are held constant at zero:
\begin{equation}
  \label{eqn:classical_dop_deform_grad}
  \epsilon|^{dop} = 0
\end{equation}
$\epsilon|^{dop}$ is the strain tensor diagonal component for the
direction of the out-of-plane strain.

### Plane Stress and Generalized Plane Strain

In the cases of the plane stress and generalized plane strain assumptions, the
component of strain and the deformation gradient in the out-of-plane direction
is non-zero. To solve for this out-of-plane strain, we use the out-of-plane
strain variable as the strain tensor component
\begin{equation}
  \label{eqn:dop_deform_grad}
  \epsilon|^{dop} = \epsilon|^{op}
\end{equation}
where $\epsilon|^{dop}$ is the strain tensor diagonal component for
the direction of the out-of-plane strain and $\epsilon|^{op}$ is a
prescribed out-of-plane strain value: this strain value can be
given either as a scalar variable or a nonlinear field variable.
The [Generalized Plane Strain](tensor_mechanics/generalized_plane_strain.md)
problems use scalar variables. Multiple scalar variables can be provided such
that one strain calculator is needed for multiple generalized plane strain
models on different subdomains.

For the case of plane stress, the [WeakPlaneStress](WeakPlaneStress.md) kernel
is used to integrate the out-of-plane component of the stress over the area of
each element, and assemble that integral to the residual of the out-of-plane
strain field variable. This results in a weak enforcement of the condition that
the out-of-plane stress is zero, which allows for re-use of the same constitutive
models for models of all dimensionality.

## Strain and Deformation Gradient Formulation

The definition of a small total linearized strain is
\begin{equation}
  \label{eqn:def_small_total_strain}
  \epsilon_{ij} = \frac{1}{2} \left( u_{i,j} + u_{j,i}  \right)
\end{equation}
The values of each of the strain tensor components depends on the direction
selected by the user as the out-of-plane direction.

#### $Z$-Direction of Out-of-Plane Strain (Default)

The default out-of-plane direction is along the $z$-axis. For this direction
the strain tensor, [eqn:def_small_total_strain], is given as
\begin{equation}
  \label{eqn:strain_tensor}
  \boldsymbol{\epsilon} = \begin{bmatrix}
                u_{x,x} & \frac{1}{2} \left(u_{x,y} + u_{y,x} \right) & 0 \\
                \frac{1}{2} \left(u_{x,y} + u_{y,x} \right) & u_{y,y} & 0 \\
                0 & 0 & \epsilon|^{dop}
              \end{bmatrix}
\end{equation}
where $\epsilon|^{dop}$ is defined in [eqn:dop_deform_grad].
As in the classical presentation of the strain tensor in plane
strain problems, the components of the strain tensor associated
with the $z$-direction are zero; these zero components indicate no
coupling between the in-plane and the out-of-plane strains.

#### $X$-Direction of Out-of-Plane Strain

If the user selects the out-of-plane direction as along the
$x$-direction, the strain tensor from [eqn:def_small_total_strain]
is given as
\begin{equation}
  \label{eqn:deform_grads_xdirs}
  \boldsymbol{\epsilon} = \begin{bmatrix}
                \epsilon|^{dop} & 0 & 0 \\
                0 & u_{y,y} & \frac{1}{2} \left(u_{y,z} + u_{z,y} \right) \\
                0 & \frac{1}{2} \left(u_{y,z} + u_{z,y} \right) & u_{z,z}
              \end{bmatrix}
\end{equation}
so that the off-diagonal components of the strain tensor associated
with the $x$-direction are zeros.

#### $Y$-Direction of Out-of-Plane Strain

If the user selects the out-of-plane direction as along the
$y$-direction, the strain tensor from [eqn:def_small_total_strain]
is given as
\begin{equation}
  \label{eqn:deform_grads_ydirs}
  \boldsymbol{\epsilon} = \begin{bmatrix}
                u_{x,x} & 0 & \frac{1}{2} \left(u_{x,z} + u_{z,x} \right) \\
                0 & \epsilon|^{dop} & 0 \\
                \frac{1}{2} \left(u_{x,z} + u_{z,x} \right) & 0 & u_{z,z}
              \end{bmatrix}
\end{equation}
so that the off-diagonal components of the strain tensor associated
with the $y$-direction are zeros.

### Volumetric Locking Correction for Strain Tensor

If selected by the user, the strain tensor is conditioned with
a $\bar{B}$ formulation to mitigate volumetric locking of the elements.
The volumetric locking correction is applied to the total strain
\begin{equation}
  \label{eqn:vlc_strain}
  \boldsymbol{\epsilon}|_{vlc} = \boldsymbol{\epsilon} + \frac{\left( \boldsymbol{\epsilon}_V - tr(\boldsymbol{\epsilon}) \right)}{3} \cdot \boldsymbol{I}
\end{equation}
where $\boldsymbol{\epsilon}_V$ is the volumetric strain and $\boldsymbol{I}$
is the Rank-2 identity tensor. For more details about the theory
behind [eqn:vlc_strain] see the
[Volumetric Locking Correction](/tensor_mechanics/VolumetricLocking.md)
documentation.

## Example Input Files

### Plane Stress

The tensor mechanics [Master action](/Modules/TensorMechanics/Master/index.md)
can be used to create the `ComputePlaneSmallStrain` class by setting
`planar_formulation = WEAK_PLANE_STRESS` and `strain = SMALL` in the
`Master` action block.

!listing modules/tensor_mechanics/test/tests/plane_stress/weak_plane_stress_small.i block=Modules/TensorMechanics/Master

Note that for plane stress analysis, the `out_of_plane_strain` parameter must be
defined, and is the name of the out-of-plane strain field variable.

!listing modules/tensor_mechanics/test/tests/plane_stress/weak_plane_stress_small.i block=Variables/strain_zz

In the case of this example, `out_of_plane_strain` is defined in the `GlobalParams` block.

### Generalized Plane Strain

The use of this plane strain class for
[Generalized Plane Strain](tensor_mechanics/generalized_plane_strain.md)
simulations uses the scalar out-of-plane strains. The tensor mechanics
[Master action](/Modules/TensorMechanics/Master/index.md) is used to create the
`ComputePlaneSmallStrain` class with the `planar_formulation = GENERALIZED_PLANE_STRAIN`
and `strain = SMALL` settings.

!listing modules/tensor_mechanics/test/tests/generalized_plane_strain/generalized_plane_strain_small.i block=Modules/TensorMechanics/Master/all

Note that the argument for the `scalar_out_of_plane_strain` parameter is the
name of the scalar strain variable

!listing modules/tensor_mechanics/test/tests/generalized_plane_strain/generalized_plane_strain_small.i block=Variables/scalar_strain_zz

### $Y$-Direction of Out-of-Plane Strain

This plane strain class is used to model plane strain with an out-of-plane strain
in directions other than in the $z$-direction. As an example, the tensor mechanics
[Master action](/Modules/TensorMechanics/Master/index.md) can be used to create
the `ComputePlaneFiniteStrain` class for a $y$-direction out-of-plane strain with
the `planar_formulation = PLANE_STRAIN` and the `out_of_plane_direction = y`
settings.

!listing modules/tensor_mechanics/test/tests/2D_different_planes/planestrain_xz.i block=Modules/TensorMechanics/Master/plane_strain

!syntax parameters /Materials/ComputePlaneSmallStrain

!syntax inputs /Materials/ComputePlaneSmallStrain

!syntax children /Materials/ComputePlaneSmallStrain

!bibtex bibliography
