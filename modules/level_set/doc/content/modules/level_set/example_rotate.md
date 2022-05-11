# Rotating Circle

The second example is a typical benchmark problem for the level set equation: a rotating bubble. The
problem involves initializing $u_h$ (see [Theory](/level_set/theory.md)) with a "bubble" of radius 0.15 at
$(0.0, 0.5)$ for $\Omega = [-1,1]^2$.  This bubble is then advected with the given velocity field
$\vec{v}=(4y,-4x)$, so that, at $t=\pi/2$, the bubble should return to its original position.


## Level Set Equation

[circle_rotate_out] show the results of solving the rotating bubble problem with the level set
equation alone, which initially behaves in a consistent manner. However, near the end of the
simulation node-to-node oscillations appear in the solution, which is evident in the contour lines
shown in [circle_rotate_out].  As expected, these oscillations influence the area of the region
encapsulated by the 0.5 level set contour as shown in [fig:area-compare] and discussed in the
[Area Comparison](#area-comparison) section.

The complete input file for running this portion of the example is included below and it may be
executed as follows.

```bash
cd ~/projects/moose/module/level_set/examples/rotating_circle
../../level_set-opt -i rotating_circle.i
```

!listing modules/level_set/examples/rotating_circle/circle_rotate.i

## Level Set Equation with SUPG

Adding SUPG stabilization---set the [theory](/level_set/theory.md) for details---mitigates the oscillations
present in the first step, as shown in [circle_rotate_supg_out]. Adding the SUPG stabilization is
trivial simply add the time and advection SUPG kernels to the input file ([circle_rotate_supg.i])
shown previously, the kernels block will then appear as:

!listing modules/level_set/examples/rotating_circle/circle_rotate_supg.i block=Kernels

Adding the stabilization improve the numerical solution but it suffers from a loss of conservation of
the phase field variable, as discussed in the [Area Comparison](#area-comparison) section and shown
in [fig:area-compare].

## Level Set Equation with Reinitialization

Adding reinitializtion, in this case the scheme proposed by [!cite](olsson2007conservative), requires
the use of the MOOSE [MultiApp](/MultiApps/index.md). The enable reinitialization two input files are
required: a parent and sub-application.

The parent input file must add the necessary [MultiApps](/MultiApps/index.md) and
[Transfers](/Transfers/index.md) blocks. For the problem at hand ([circle_rotate_parent.i]) this
easily accomplished by adding the following to the input file from the first step (i.e., do not
include the SUPG kernels).

!listing modules/level_set/examples/rotating_circle/circle_rotate_parent.i
         start=[MultiApps]
         end=[Outputs]

Next, the sub-application input file must be created, which is shown below. This input file mimics
the parent input file closely, with three notable exceptions. First, the
[Kernels](syntax/Kernels/index.md) block utilize the time derivative and a new object,
[LevelSetOlssonReinitialization](/LevelSetOlssonReinitialization.md), that implements the
reinitialization scheme of [!cite](olsson2007conservative). Second, the [Problem](/Problem/index.md) is
set to use the [LevelSetReinitializationProblem](/LevelSetReinitializationProblem.md). Finally, the
[UserObjects](/UserObjects/index.md) block includes a terminator,
[LevelSetOlssonTerminator](/LevelSetOlssonTerminator.md), which is responsible for stopping the
reinitialization solve when steady-state is achieved according to the criteria defined by
[!cite](olsson2007conservative).

!listing modules/level_set/examples/rotating_circle/circle_rotate_sub.i

[circle_rotate_parent_out] shows the results of the bubble problem with reinitialization, the result
looks similar to the SUPG result. However, if you consider the area conservation discussed in the
[Area Comparison](#area-comparison) section, the reinitialization scheme yields the superior solution
for this problem.

!media level_set/circle_rotate_out.mp4
       id=circle_rotate_out
       style=width:32%;margin-right:2%;float:left;
       caption=Results from solving the rotating circle problem with the level set equation alone.

!media level_set/circle_rotate_supg_out.mp4
       id=circle_rotate_supg_out
       style=width:32%;margin-right:2%;float:left;
       caption=Results from solving the rotating circle problem with the level set equation using
               SUPG stabilization.

!media level_set/circle_rotate_master_out.mp4
       id=circle_rotate_parent_out
       style=width:32%;float:left;
       caption=Results from solving the rotating circle problem with the level set equation with
               reinitialization.

## Area Comparison id=area-comparison

[fig:area-compare] is a plot of the area of the circle during the three simulations. Note that in the
unstabilized, un-reinitialized level set equation, both area conservation and stability issues are
readily apparent. The instabilities are especially obvious in [fig:area-compare], where the drastic
area changes are due to numerical oscillations in the solution field. Adding SUPG stabilization helps
ameliorate the stability concern but it suffers from loss of area conservation. The re-initialization
scheme is both stable and area-conserving.

!media level_set/area_comparison.png
       id=fig:area-compare
       style=width:50%;float:right;margin-left:20px;
       caption=Comparison of area inside the bubble during simulations.

The re-initialization methods performs well, but it is computationally expensive and picking the
pseudo timestep size $\Delta \tau$, steady-state criteria $\delta$, and interface thickness
$\epsilon$ correctly for the re-initialization problem is a non-trivial difficulty. Nevertheless, the
level set module provides a valuable starting point and proof-of-concept for other researchers
interested in the method, and the existing algorithm can no doubt be tuned to the needs of specific
applications in terms of conservation and computational cost requirements.



!bibtex bibliography
