# ADComputeAxisymmetric1DFiniteStrain

!syntax description /Materials/ADComputeAxisymmetric1DFiniteStrain

## Description

`ADComputeAxisymmetric1DFiniteStrain` computes the finite strain increment and
rotation increment for a 1D axisymmetric generalized plane strain model using
automatic differentiation. It is the AD counterpart to
[ComputeAxisymmetric1DFiniteStrain.md].

This material assumes symmetry about the $z$ axis and must be used on blocks
with `COORD_TYPE = RZ`. The radial coordinate is the physical $x$ coordinate.
The radial deformation component is stored in `xx`, the generalized plane
strain component is stored in `yy`, and the hoop deformation component is
stored in `zz`.

The out-of-plane strain can be supplied by either
[!param](/Materials/ADComputeAxisymmetric1DFiniteStrain/scalar_out_of_plane_strain)
or [!param](/Materials/ADComputeAxisymmetric1DFiniteStrain/out_of_plane_strain),
but not both. With
[!param](/Materials/ADComputeAxisymmetric1DFiniteStrain/scalar_out_of_plane_strain),
the coupled scalar variable commonly comes from a
[Generalized Plane Strain](solid_mechanics/generalized_plane_strain.md)
model. If multiple scalar components are coupled,
[!param](/Materials/ADComputeAxisymmetric1DFiniteStrain/subblock_index_provider)
selects the component for the current element; otherwise component 0 is used.

The finite-strain deformation components are
\begin{equation}
  A_{rr} = u_{r,r}, \qquad
  A_{zz} = \exp \left( \epsilon^{op} \right) - 1, \qquad
  A_{\theta \theta} = \frac{u_r}{X_r},
\end{equation}
where $\epsilon^{op}$ is the supplied out-of-plane strain. The out-of-plane
component uses $\exp(\epsilon^{op}) - 1$.

## Example Input File Syntax

The following generalized plane strain test shows the corresponding non-AD
material block using a scalar out-of-plane strain. The same strain block
parameters apply to `ADComputeAxisymmetric1DFiniteStrain`.

!listing modules/solid_mechanics/test/tests/1D_axisymmetric/axisymm_gps_finite.i block=Materials/strain

!syntax parameters /Materials/ADComputeAxisymmetric1DFiniteStrain

!syntax inputs /Materials/ADComputeAxisymmetric1DFiniteStrain

!syntax children /Materials/ADComputeAxisymmetric1DFiniteStrain
