# ADComputeAxisymmetric1DSmallStrain

!syntax description /Materials/ADComputeAxisymmetric1DSmallStrain

## Description

`ADComputeAxisymmetric1DSmallStrain` computes the total small strain for a 1D
axisymmetric generalized plane strain model using automatic differentiation.
It is the AD counterpart to [ComputeAxisymmetric1DSmallStrain.md].

This material assumes symmetry about the $z$ axis and must be used on blocks
with `COORD_TYPE = RZ`. The radial coordinate is the physical $x$ coordinate.
The radial strain is stored in the `xx` component, the generalized plane strain
component is stored in `yy`, and the hoop strain is stored in `zz`.

The out-of-plane strain can be supplied by either
[!param](/Materials/ADComputeAxisymmetric1DSmallStrain/scalar_out_of_plane_strain)
or [!param](/Materials/ADComputeAxisymmetric1DSmallStrain/out_of_plane_strain),
but not both. With
[!param](/Materials/ADComputeAxisymmetric1DSmallStrain/scalar_out_of_plane_strain),
the coupled scalar variable commonly comes from a
[Generalized Plane Strain](solid_mechanics/generalized_plane_strain.md)
model. If multiple scalar components are coupled,
[!param](/Materials/ADComputeAxisymmetric1DSmallStrain/subblock_index_provider)
selects the component for the current element; otherwise component 0 is used.

The small-strain components are
\begin{equation}
  \epsilon_{rr} = u_{r,r}, \qquad
  \epsilon_{zz} = \epsilon^{op}, \qquad
  \epsilon_{\theta \theta} = \frac{u_r}{X_r},
\end{equation}
where $\epsilon^{op}$ is the supplied out-of-plane strain.

## Example Input File Syntax

The following generalized plane strain test shows the corresponding non-AD
material block using a scalar out-of-plane strain. The same strain block
parameters apply to `ADComputeAxisymmetric1DSmallStrain`.

!listing modules/solid_mechanics/test/tests/1D_axisymmetric/axisymm_gps_small.i block=Materials/strain

!syntax parameters /Materials/ADComputeAxisymmetric1DSmallStrain

!syntax inputs /Materials/ADComputeAxisymmetric1DSmallStrain

!syntax children /Materials/ADComputeAxisymmetric1DSmallStrain
