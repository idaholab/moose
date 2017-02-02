# Vortex
A classic and challenging benchmark problem for the level set is the vortex problem. This problem is similar to the
rotating bubble (see [Rotating Bubble Example](level_set/example_rotate.md)). The problem involves initializing $u_h$ (see [Theory](level_set/theory.md)) with a "bubble" of
radius 0.15 at $(0.5, 0.75)$ for $\Omega = [0,1]^2$.  This bubble is
then advected with the a vortex velocity field (see [LevelSetOlssonVortex](level_set/LevelSetOlssonVortex.md)), so that,
at $t=2$, the bubble should return to its original position.

The only significant difference between the [Rotating Bubble Example](level_set/example_rotate.md) and this example
is the velocity function; therefore, a detailed discussion of the varying input files is omitted.

As is the case for the the [Rotating Bubble Example](level_set/example_rotate.md) three scenarios are considered: the
level set equation alone, with SUPG stabilization, and with reinitialization. The complete input files are located
in the `modules/level_set/examples/vortex` directory of MOOSE.

Figures \ref{vortex_out} through \ref{vortex_reinit_out} show the results of the three scenarios. The solution quality
of the reinitialization case (Figure \ref{vortex_reinit_out}) is obviously lacking. The reasoning the solution is
behaving in the manner is not fully understood, with respect to the level set module. This is an something that needs
to be investigated further.

!figure docs/media/level_set/vortex_out.gif id=vortex_out width=32% margin-right=2% float=left caption=Results from solving the vortex problem with the level set equation alone.

!figure docs/media/level_set/vortex_out.gif id=vortex_supg_out width=32% margin-right=2% float=left caption=Results from solving the vortex problem with the level set equation using SUPG stabilization.

!figure docs/media/level_set/vortex_reinit_out.gif id=vortex_reinit_out width=32% float=left caption=Results from solving the vortex problem with the level set equation with reinitialization.

The area comparison of the three methods is shown in Figure \ref{area_comparison}, again the reinitialization example
is not performing as expected. The area results shown here indicates that reinitialization step as implemented in
this module may be lacking. Additional work should be done to add additional stabilization and reinitialization schemes
and uncover the inconsistencies seen here.

!figure docs/media/level_set/example_vortex_area.png id=area_comparison caption=Comparison of area inside the vortex during simulations. float=right width=100%
