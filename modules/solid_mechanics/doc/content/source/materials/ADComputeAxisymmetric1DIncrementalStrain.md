# ADComputeAxisymmetric1DIncrementalStrain

!syntax description /Materials/ADComputeAxisymmetric1DIncrementalStrain

## Description

`ADComputeAxisymmetric1DIncrementalStrain` computes the small strain increment
for a 1D axisymmetric generalized plane strain model using automatic
differentiation. It is the AD counterpart to
[ComputeAxisymmetric1DIncrementalStrain.md].

This material assumes symmetry about the $z$ axis and must be used on blocks
with `COORD_TYPE = RZ`. The radial coordinate is the physical $x$ coordinate.
The radial strain increment is stored in the `xx` component, the generalized
plane strain increment is stored in `yy`, and the hoop strain increment is
stored in `zz`.

The out-of-plane strain can be supplied by either
[!param](/Materials/ADComputeAxisymmetric1DIncrementalStrain/scalar_out_of_plane_strain)
or [!param](/Materials/ADComputeAxisymmetric1DIncrementalStrain/out_of_plane_strain),
but not both. With
[!param](/Materials/ADComputeAxisymmetric1DIncrementalStrain/scalar_out_of_plane_strain),
the coupled scalar variable commonly comes from a
[Generalized Plane Strain](solid_mechanics/generalized_plane_strain.md)
model. If multiple scalar components are coupled,
[!param](/Materials/ADComputeAxisymmetric1DIncrementalStrain/subblock_index_provider)
selects the component for the current element; otherwise component 0 is used.

The increment is formed from the difference between current AD values and old
state values. The strain-expression components are
\begin{equation}
  \epsilon_{rr} = u_{r,r}, \qquad
  \epsilon_{zz} = \epsilon^{op}, \qquad
  \epsilon_{\theta \theta} = \frac{u_r}{X_r},
\end{equation}
where $\epsilon^{op}$ is the supplied out-of-plane strain.

## Example Input File Syntax

The following generalized plane strain test shows the corresponding non-AD
material block using a scalar out-of-plane strain. The same strain block
parameters apply to `ADComputeAxisymmetric1DIncrementalStrain`.

!listing modules/solid_mechanics/test/tests/1D_axisymmetric/axisymm_gps_incremental.i block=Materials/strain

!syntax parameters /Materials/ADComputeAxisymmetric1DIncrementalStrain

!syntax inputs /Materials/ADComputeAxisymmetric1DIncrementalStrain

!syntax children /Materials/ADComputeAxisymmetric1DIncrementalStrain
