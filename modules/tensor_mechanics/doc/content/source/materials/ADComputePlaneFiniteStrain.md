# Compute Plane Finite Strain

!syntax description /Materials/ADComputePlaneFiniteStrain


## Description

The material `ADComputePlaneFiniteStrain` calculates the finite strain for 2D
plane strain problems. It can be used for classical
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
  F|^{dop} = 0 \text{  and  } \epsilon|^{dop} = 0
\end{equation}
$F|^{dop}$ is the deformation gradient tensor diagonal component for the
direction of the out-of-plane strain and $\epsilon|^{dop}$ is the corresponding
strain component.

### Plane Stress and Generalized Plane Strain

In the cases of the plane stress and generalized plane strain assumptions, the
component of strain and the deformation gradient in the out-of-plane direction
is non-zero. To solve for this out-of-plane strain, we invoke the approximation
of the stretch rate tensor
\begin{equation}
  \label{eqn:stretch_tensor_approx}
  \boldsymbol{D} = \log \left( \sqrt{\hat{\boldsymbol{F}}^T \cdot \hat{\boldsymbol{F}}} \right) \cdot \frac{1}{dt}
\end{equation}
and define the deformation gradient component in the out-of-plane direction as
\begin{equation}
  \label{eqn:dop_deform_grad}
  F|^{dop} = \exp \left( \epsilon|^{op} - 1.0  \right)
\end{equation}
where $F|^{dop}$ is the deformation gradient tensor diagonal component for the
direction of the out-of-plane strain and $\epsilon|^{op}$ is a prescribed
out-of-plane strain value: this strain value can be given either as a scalar
variable or a nonlinear field variable.

For the case of plane stress, the [ADWeakPlaneStress](ADWeakPlaneStress.md) kernel
is used to integrate the out-of-plane component of the stress over the area of
each element, and assemble that integral to the residual of the out-of-plane
strain field variable. This results in a weak enforcement of the condition that
the out-of-plane stress is zero, which allows for re-use of the same constitutive
models for models of all dimensionality.

The [Generalized Plane Strain](tensor_mechanics/generalized_plane_strain.md)
problems use scalar variables. Multiple scalar variables can be provided such
that one strain calculator is needed for multiple generalized plane strain
models on different subdomains.


## Strain and Deformation Gradient Formulation

The incremental deformation gradient for the 2D planar system is defined as
\begin{equation}
  \label{eqn:incremental_deformation_grad}
  \hat{\boldsymbol{F}} = \boldsymbol{A} : \bar{\boldsymbol{F}}^{-1} + \boldsymbol{I}
\end{equation}
where $\boldsymbol{I}$ is the Rank-2 identity tensor, $\boldsymbol{A}$ is the deformation
gradient, and $\bar{\boldsymbol{F}}$ is the old deformation gradient.

#### $Z$-Direction of Out-of-Plane Strain (Default)

The default out-of-plane direction is along the $z$-axis. For this direction
the current and old deformation gradient tensors, used in
[eqn:incremental_deformation_grad], are given as
\begin{equation}
  \label{eqn:deform_grads}
  \boldsymbol{A} = \begin{bmatrix}
                u_{x,x} & u_{x,y} & 0 \\
                u_{y,x} & u_{y,y} & 0 \\
                0 & 0 & F|^{dop}
              \end{bmatrix} + \boldsymbol{I}
  \qquad \text{  and  } \qquad
  \bar{\boldsymbol{F}} = \begin{bmatrix}
                u_{x,x}|_{old} & u_{x,y}|_{old} & 0 \\
                u_{y,x}|_{old} & u_{y,y}|_{old} & 0 \\
                0 & 0 & F|^{dop}_{old}
              \end{bmatrix} + \boldsymbol{I}
\end{equation}
where $F|^{dop}$ is defined in [eqn:dop_deform_grad].
Note that $\bar{\boldsymbol{F}}$ uses the values of the strain expressions from
the previous time step.
As in the classical presentation of the strain tensor in plane strain problems,
the components of the deformation tensor associated with the $z$-direction are
zero; these zero components indicate no coupling between the in-plane displacements
and the out-of-plane strain variable.

#### $X$-Direction of Out-of-Plane Strain

If the user selects the out-of-plane direction as along the $x$-direction, the
current and old deformation gradient tensors from [eqn:incremental_deformation_grad]
are formulated as
\begin{equation}
  \label{eqn:deform_grads_xdirs}
  \boldsymbol{A} = \begin{bmatrix}
                F|^{dop} & 0 & 0 \\
                0 & u_{y,y} & u_{z,y} \\
                0 & u_{y,z} & u_{z,z}
              \end{bmatrix} + \boldsymbol{I}
  \qquad \text{  and  } \qquad
  \bar{\boldsymbol{F}} = \begin{bmatrix}
                F|^{dop}_{old} & 0 & 0 \\
                0 & u_{y,y}|_{old} & u_{y,z}|_{old} \\
                0 & u_{z,y}|_{old}& u_{z,z}|_{old}
              \end{bmatrix} + \boldsymbol{I}
\end{equation}
so that the off-diagonal components of the deformation tensors associated with
the $x$-direction are zeros.

#### $Y$-Direction of Out-of-Plane Strain

If the user selects the out-of-plane direction as along the $y$-direction, the
current and old deformation gradient tensors from [eqn:incremental_deformation_grad]
are formulated as
\begin{equation}
  \label{eqn:deform_grads_ydirs}
  \boldsymbol{A} = \begin{bmatrix}
                u_{x,x} & 0 & u_{z,x} \\
                0 & F|^{dop} & 0 \\
                u_{x,z} & 0 & u_{z,z}
              \end{bmatrix} + \boldsymbol{I}
  \qquad \text{  and  } \qquad
  \bar{\boldsymbol{F}} = \begin{bmatrix}
                u_{x,x}|_{old} & 0 & u_{x,z}|_{old} \\
                0 & F|^{dop}_{old} & 0 \\
                u_{z,x}|_{old} & 0 & u_{z,z}|_{old}
              \end{bmatrix} + \boldsymbol{I}
\end{equation}
so that the off-diagonal components of the deformation tensors associated with
the $y$-direction are zeros.


### Finalized Deformation Gradient

If selected by the user, the incremental deformation gradient is conditioned with
a $\bar{B}$ formulation to mitigate volumetric locking of the elements.
The volumetric locking correction is applied to both the incremental deformation
gradient
\begin{equation}
  \label{eqn:vlc_fhat}
  \hat{\boldsymbol{F}}|_{vlc} = \left( \frac{1}{det(\hat{\boldsymbol{F}})} \frac{\hat{\boldsymbol{F}}_{avg}}{V_{elem}} \right)^{1/3}
\end{equation}
and the total deformation gradient. For more details about the theory behind
[eqn:vlc_fhat] see the
[Volumetric Locking Correction](/tensor_mechanics/VolumetricLocking.md)
documentation.

Once the incremental deformation gradient is calculated for the specific 2D geometry,
the deformation gradient is passed to the strain and rotation methods used by the
3D Cartesian simulations, as described in the [Finite Strain Class](ADComputeFiniteStrain.md)
documentation.

## Example Input Files

### Plane Stress

The tensor mechanics [Master action](/Modules/TensorMechanics/Master/index.md)
can be used to create the `ADComputePlaneFiniteStrain` class by setting
`planar_formulation = WEAK_PLANE_STRESS` and `strain = FINITE` in the
`Master` action block.

!listing modules/tensor_mechanics/test/tests/plane_stress/weak_plane_stress_finite.i block=Modules/TensorMechanics/Master

Note that for plane stress analysis, the `out_of_plane_strain` parameter must be
defined, and is the name of the out-of-plane strain field variable.

!listing modules/tensor_mechanics/test/tests/plane_stress/weak_plane_stress_finite.i block=Variables/strain_zz

In the case of this example, `out_of_plane_strain` is defined in the `GlobalParams` block.

### Generalized Plane Strain

The use of this plane strain class for
[Generalized Plane Strain](tensor_mechanics/generalized_plane_strain.md)
simulations uses the scalar out-of-plane strains. The tensor mechanics
[Master action](/Modules/TensorMechanics/Master/index.md) is used to create the
`ADComputePlaneFiniteStrain` class with the `planar_formulation = GENERALIZED_PLANE_STRAIN`
and the `strain = FINITE` settings.

!listing modules/tensor_mechanics/test/tests/generalized_plane_strain/generalized_plane_strain_finite.i block=Modules/TensorMechanics/Master/all

Note that the argument for the `scalar_out_of_plane_strain` parameter is the
name of the scalar strain variable

!listing modules/tensor_mechanics/test/tests/generalized_plane_strain/generalized_plane_strain_finite.i block=Variables/scalar_strain_zz

!syntax parameters /Materials/ADComputePlaneFiniteStrain

!syntax inputs /Materials/ADComputePlaneFiniteStrain

!syntax children /Materials/ADComputePlaneFiniteStrain

!bibtex bibliography
