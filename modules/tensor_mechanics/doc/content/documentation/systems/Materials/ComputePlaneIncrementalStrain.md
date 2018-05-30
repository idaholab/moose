# Compute Plane Incremental Strain

!syntax description /Materials/ComputePlaneIncrementalStrain

## Description

The material `ComputePlaneIncrementalStrain` calculates the small incremental
strain for 2D plane strain problems. It can be used for  classical
[thick body plane strain](https://en.wikipedia.org/wiki/Plane_stress)
problems, [Weak Plane Stress](Kernels/WeakPlaneStress.md) models, or in
[Generalized Plane Strain](tensor_mechanics/generalized_plane_strain.md) simulations.

## Out of Plane Strain

In the classical thick body plane strain problem, the strain and deformation
gradient components in the out-of-plane direction (the infinitely thick
direction) are held constant at zero:
\begin{equation}
  \label{eqn:classical_dop_deform_grad}
  F|^{dop} = 0 \text{  and  } \epsilon|^{dop} = 0
\end{equation}
$F|^{dop}$ is the deformation gradient tensor diagonal component for the
direction of the out-of-plane strain and $\epsilon|^{dop}$ is the corresponding
strain component.

### Generalized Plane Strain

In the cases of the generalized plane strain and weak plane stress models, the
component of strain and the deformation gradient in the out-of-plane direction
is non-zero. To solve for this out-of-plane strain, we use the out-of-plane
strain variable as the deformation gradient component
\begin{equation}
  \label{eqn:dop_deform_grad}
  F|^{dop} = \epsilon|^{op}
\end{equation}
where $F|^{dop}$ is the deformation gradient tensor diagonal component for the
direction of the out-of-plane strain and $\epsilon|^{op}$ is a prescribed
out-of-plane strain value: this strain value can be given either as a scalar
variable or a nonlinear variable.
The [Generalized Plane Strain](tensor_mechanics/generalized_plane_strain.md)
problems use scalar variables.


## Strain and Deformation Gradient Formulation

The small strain increment is calculated with the form
\begin{equation}
  \label{eqn:strain_increment}
  \Delta \mathbf{\epsilon} = \frac{1}{2} \left( \mathbf{D} + \mathbf{D}^T \right)
  \text{ where } \mathbf{D} = \mathbf{A} - \bar{\mathbf{F}} + \mathbf{I}
\end{equation}
where $\mathbf{I}$ is the Rank-2 identity tensor, $\mathbf{A}$ is the deformation
gradient, and $\bar{\mathbf{F}}$ is the old deformation gradient.

#### $Z$-Direction of Out-of-Plane Strain (Default)

The default out-of-plane direction is along the $z$-axis. For this direction
the current and old deformation gradient tensors, used in
[eqn:strain_increment], are given as
\begin{equation}
  \label{eqn:deform_grads}
  \mathbf{A} = \begin{bmatrix}
                u_{x,x} & u_{x,y} & 0 \\
                u_{y,x} & u_{y,y} & 0 \\
                0 & 0 & F|^{dop}
              \end{bmatrix} + \mathbf{I}
  \qquad \text{  and  } \qquad
  \bar{\mathbf{F}} = \begin{bmatrix}
                u_{x,x}|_{old} & u_{x,y}|_{old} & 0 \\
                u_{y,x}|_{old} & u_{y,y}|_{old} & 0 \\
                0 & 0 & F|^{dop}_{old}
              \end{bmatrix} + \mathbf{I}
\end{equation}
where $F|^{dop}$ is defined in [eqn:dop_deform_grad].
Note that $\bar{\mathbf{F}}$ uses the values of the strain expressions from
the previous time step.
As in the classical presentation of the strain tensor in plane strain problems,
the components of the deformation tensor associated with the $z$-direction are
zero; these zero components indicate no coupling between the in-plane displacements
and the out-of-plane strain variable.

#### $X$-Direction of Out-of-Plane Strain

If the user selects the out-of-plane direction as along the $x$-direction, the
current and old deformation gradient tensors from [eqn:strain_increment] are
formulated as
\begin{equation}
  \label{eqn:deform_grads_xdirs}
  \mathbf{A} = \begin{bmatrix}
                F|^{dop} & 0 & 0 \\
                0 & u_{y,y} & u_{z,y} \\
                0 & u_{y,z} & u_{z,z}
              \end{bmatrix} + \mathbf{I}
  \qquad \text{  and  } \qquad
  \bar{\mathbf{F}} = \begin{bmatrix}
                F|^{dop}_{old} & 0 & 0 \\
                0 & u_{y,y}|_{old} & u_{y,z}|_{old} \\
                0 & u_{z,y}|_{old}& u_{z,z}|_{old}
              \end{bmatrix} + \mathbf{I}
\end{equation}
so that the off-diagonal components of the deformation tensors associated with
the $x$-direction are zeros.

#### $Y$-Direction of Out-of-Plane Strain

If the user selects the out-of-plane direction as along the $y$-direction, the
current and old deformation gradient tensors from [eqn:strain_increment] are
formulated as
\begin{equation}
  \label{eqn:deform_grads_ydirs}
  \mathbf{A} = \begin{bmatrix}
                u_{x,x} & 0 & u_{z,x} \\
                0 & F|^{dop} & 0 \\
                u_{x,z} & 0 & u_{z,z}
              \end{bmatrix} + \mathbf{I}
  \qquad \text{  and  } \qquad
  \bar{\mathbf{F}} = \begin{bmatrix}
                u_{x,x}|_{old} & 0 & u_{x,z}|_{old} \\
                0 & F|^{dop}_{old} & 0 \\
                u_{z,x}|_{old} & 0 & u_{z,z}|_{old}
              \end{bmatrix} + \mathbf{I}
\end{equation}
so that the off-diagonal components of the deformation tensors associated with
the $y$-direction are zeros.

### Finalized Deformation Gradient

Once the incremental deformation gradient is calculated for the specific 2D geometry,
the deformation gradient is passed to the strain and rotation methods used by the
3D Cartesian simulations, as described in the
[Compute Incremental Small Strain](ComputeIncrementalSmallStrain.md) documentation.

If selected by the user, the incremental strain tensor is conditioned with
a $\bar{B}$ formulation to mitigate volumetric locking of the elements.
The volumetric locking correction is applied to the total strain
\begin{equation}
  \label{eqn:vlc_strain}
  \Delta \mathbf{\epsilon}|_{vlc} = \mathbf{\epsilon} + \frac{\left( \mathbf{\epsilon}_V - tr(\mathbf{\Delta \epsilon}) \right)}{3} \cdot \mathbf{I}
\end{equation}
where $\mathbf{\epsilon}_V$ is the volumetric strain and $\mathbf{I}$
is the Rank-2 identity tensor. For more details about the theory
behind [eqn:vlc_strain] see the
[Volumetric Locking Correction](/tensor_mechanics/VolumetricLocking.md)
documentation.


## Example Input File

### Generalized Plane Strain

As an example, the use of this plane strain class with the
[Generalized Plane Strain](tensor_mechanics/generalized_plane_strain.md)
simulations uses the scalar out-of-plane strains. The tensor mechanics
[MasterAction](/Modules/TensorMechanics/Master/index.md) is used to create the
`ComputePlaneIncrementalStrain` class with the `planar_formulation = GENERALIZED_PLANE_STRAIN`,
`strain = SMALL`, and `incremental = true` settings.

!listing modules/tensor_mechanics/test/tests/generalized_plane_strain/generalized_plane_strain_increment.i block=Modules/TensorMechanics/Master/all

Note that the argument for the `scalar_out_of_plane_strain` parameter is the
name of the scalar strain variable

!listing modules/tensor_mechanics/test/tests/generalized_plane_strain/generalized_plane_strain_increment.i block=Variables/scalar_strain_zz


!syntax parameters /Materials/ComputePlaneIncrementalStrain

!syntax inputs /Materials/ComputePlaneIncrementalStrain

!syntax children /Materials/ComputePlaneIncrementalStrain

!bibtex bibliography
