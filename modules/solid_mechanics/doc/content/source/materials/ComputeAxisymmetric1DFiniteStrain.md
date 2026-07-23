# ComputeAxisymmetric1DFiniteStrain / ADComputeAxisymmetric1DFiniteStrain

!syntax description /Materials/ComputeAxisymmetric1DFiniteStrain

## Description

`ComputeAxisymmetric1DFiniteStrain` computes the finite strain increment and
rotation increment for a 1D axisymmetric generalized plane strain model. When
the solid mechanics action is used with `use_automatic_differentiation = true`,
the action selects `ADComputeAxisymmetric1DFiniteStrain` for the same strain
formulation.

This material assumes symmetry about the $z$ axis and must be used on blocks
with `COORD_TYPE = RZ`. The radial coordinate is the physical $x$ coordinate.
In the 1D strain tensor, MOOSE maps the radial strain to the `rr` or `xx`
component, the generalized plane strain to the axial `yy` component, and the
hoop strain to the `zz` component. The stress divergence is handled by the
axisymmetric RZ kernels, such as [StressDivergenceRZTensors](/StressDivergenceRZTensors.md).

The out-of-plane strain can be supplied by either
[!param](/Materials/ComputeAxisymmetric1DFiniteStrain/scalar_out_of_plane_strain)
or [!param](/Materials/ComputeAxisymmetric1DFiniteStrain/out_of_plane_strain),
but not both. Scalar out-of-plane strain values are commonly used by
[Generalized Plane Strain](solid_mechanics/generalized_plane_strain.md)
models. If multiple scalar components are coupled,
[!param](/Materials/ComputeAxisymmetric1DFiniteStrain/subblock_index_provider)
selects which scalar component applies to the current element; without that
user object, component 0 is used.

For the AD object, current displacement and out-of-plane strain values are AD
values. Old state values used to form the finite strain increment remain
regular values.

## 1D Axisymmetric Strain Formulation

The axisymmetric model uses the cylindrical coordinates, $r$, $z$, and
$\theta$, where the line in the $r$ direction is rotated about the $z$ axis in
the $\theta$ direction. The incremental deformation gradient for the 1D
axisymmetric system is
\begin{equation}
  \label{eqn:incremental_deformation_grad}
  \hat{\boldsymbol{F}} = \boldsymbol{A} \cdot \bar{\boldsymbol{F}}^{-1}
\end{equation}
where $\boldsymbol{I}$ is the Rank-2 identity tensor, and the deformation
gradient, $\boldsymbol{A}$, and the old deformation gradient,
$\bar{\boldsymbol{F}}$, are
\begin{equation}
  \label{eqn:deform_grads}
  \boldsymbol{A} = \begin{bmatrix}
                \epsilon_{rr} & 0 & 0 \\
                0 & \epsilon_{zz} & 0 \\
                0 & 0 & \epsilon_{\theta \theta}
              \end{bmatrix} + \boldsymbol{I}
  \qquad \text{  and  } \qquad
  \bar{\boldsymbol{F}} = \begin{bmatrix}
                \epsilon_{rr}|_{old} & 0 & 0 \\
                0 & \epsilon_{zz}|_{old} & 0 \\
                0 & 0 & \epsilon_{\theta \theta}|_{old}
              \end{bmatrix} + \boldsymbol{I}
\end{equation}
The old deformation gradient uses strain-expression values from the previous
time step. The tensor components are
\begin{equation}
  \label{eqn:strain_components}
  \begin{aligned}
  \epsilon_{rr} & = u_{r,r} \\
  \epsilon_{zz} & = \exp \left( \epsilon|^{op} \right) - 1 \\
  \epsilon_{\theta \theta} & = \frac{u_r}{X_r}
  \end{aligned}
\end{equation}
where $\epsilon|^{op}$ is the supplied out-of-plane strain. In the MOOSE tensor
storage for this 1D formulation, the generalized plane strain component is
stored in `yy` and the hoop component is stored in `zz`. The finite-strain
out-of-plane deformation component is computed as
$\exp(\epsilon|^{op}) - 1$, not $\exp(\epsilon|^{op} - 1)$.

After the incremental deformation gradient is calculated for the 1D geometry,
the deformation gradient is passed to the strain and rotation calculations used
by the default 3D Cartesian finite strain model, as described in
[ComputeFiniteStrain.md].

## Example Input File Syntax

The following generalized plane strain test uses the scalar out-of-plane strain
option with `ComputeAxisymmetric1DFiniteStrain`.

!listing modules/solid_mechanics/test/tests/1D_axisymmetric/axisymm_gps_finite.i block=Materials/strain

The coupled scalar variable is defined in the same input file.

!listing modules/solid_mechanics/test/tests/1D_axisymmetric/axisymm_gps_finite.i block=Variables/scalar_strain_yy

!syntax parameters /Materials/ComputeAxisymmetric1DFiniteStrain

!syntax inputs /Materials/ComputeAxisymmetric1DFiniteStrain

!syntax children /Materials/ComputeAxisymmetric1DFiniteStrain
