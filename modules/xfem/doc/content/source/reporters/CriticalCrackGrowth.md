# CriticalCrackGrowth

## Description

The `CriticalCrackGrowth` Reporter computes the crack growth increment at every active crack front point provided by a [CrackMeshCut3DUserObject.md]. The effective stress intensity factor is

\begin{equation}
K_{\mathrm{eff}} = \sqrt{K_I^2 + K_{II}^2}.
\end{equation}

At crack front point $i$, the reported growth increment is

\begin{equation}
\Delta a_i =
\begin{cases}
[!param](/Reporters/CriticalCrackGrowth/max_growth_increment), & K_{\mathrm{eff},i} > K_{\mathrm{critical}} \text{ and } K_{I,i} > 0, \\
0, & \text{otherwise},
\end{cases}
\end{equation}

where $K_{\mathrm{critical}}$ is set by the required [!param](/Reporters/CriticalCrackGrowth/k_critical) parameter.

The strict toughness and positive opening-mode criteria match those used by [MeshCut2DFractureUserObject.md]. The stress intensity factors are read from the vector postprocessors selected with [!param](/Reporters/CriticalCrackGrowth/ki_vectorpostprocessor) and [!param](/Reporters/CriticalCrackGrowth/kii_vectorpostprocessor). The output vector name is set by [!param](/Reporters/CriticalCrackGrowth/growth_increment_name), and that same name must be repeated in [!param](/UserObjects/CrackMeshCut3DUserObject/growth_reporter) on the [CrackMeshCut3DUserObject.md] as `<reporter_name>/<growth_increment_name>`, as the example below does.

## Example Syntax

The test example uses a circular crack in a hex mesh and a prescribed stress field that places crack front points both above and below the critical fracture toughness over two time steps.

!listing /modules/xfem/test/tests/solid_mechanics_basic/face_crack_3d_xfem_critical_growth.i block=Reporters

!syntax parameters /Reporters/CriticalCrackGrowth

!syntax inputs /Reporters/CriticalCrackGrowth

!syntax children /Reporters/CriticalCrackGrowth
