# Rotating Circle
The second example is a typical benchmark problem for the level set equation: a rotating
bubble. The problem involves initializing $u_h$ (see [Theory](level_set/theory.md)) with a "bubble" of
radius 0.15 at $(0.0, 0.5)$ for $\Omega = [-1,1]^2$.  This bubble is
then advected with the given velocity field $\vec{v}=(4y,-4x)$, so that, at
$t=\pi/2$, the bubble should return to its original position.


## Level Set Equation

Figure \ref{circle_rotate_out} show the results of solving the rotating bubble problem with the level set equation
alone, which initially behaves in a consistent manner. However, near the end of the simulation node-to-node
oscillations appear in the solution, which is evident in the contour lines shown in Figure \ref{circle_rotate_out}.
As aspected, these oscillations influence the area of the region encapsulated by the 0.5 level set contour as
shown in Figure \ref{area_comparison} and discussed in the [Area Comparison](#area_comparison) section.

The complete input file for running this portion of the example is included below and it may be executed as follows.

```bash
cd ~/projects/moose/module/level_set/examples/rotating_circle
../../level_set-opt -i rotating_circle.i
```

!text modules/level_set/examples/rotating_circle/circle_rotate.i max-height=300px overflow-y=scroll

## Level Set Equation with SUPG
Adding SUPG stabilization---set the [theory](level_set/theory.md) for details---mitigates the oscillations present in
the first step, as shown in Figure \ref{circle_rotate_supg_out}. Adding the SUPG stabilization is trivial simply add
the time and advection SUPG kernels to the input file ([circle_rotate_supg.i](https://github.com/idaholab/moose/tree/devel/modules/level_set/examples/rotating_circle/circle_rotate_supg.i)) shown previously, the kernels block will then appear as:

!input modules/level_set/examples/rotating_circle/circle_rotate_supg.i block=Kernels label=False

Adding the stabilization improve the numerical solution but it suffers from a loss of conservation of the phase field
variable, as discussed in the [Area Comparison](#area_comparison) section and shown in Figure \ref{area_comparison}.

## Level Set Equation with Reinitialization
Adding reinitializtion, in this case the scheme proposed by \cite{olsson2007conservative}, requires the use of the
MOOSE [MultiApp](/MultiApps/index.md). The enable reinitialization two input files are required: a master and sub-application.

The master input file must add the necessary [MultiApps](/MultiApps/index.md) and [Transfers](/Transfers/index.md)
blocks. For the problem at hand ([circle_rotate_master.i](https://github.com/idaholab/moose/tree/devel/modules/level_set/examples/rotating_circle/circle_rotate_master.i)) this easily accomplished by adding the following to the input file from the first step (i.e., do not
include the SUPG kernels).

!text modules/level_set/examples/rotating_circle/circle_rotate_master.i start=[MultiApps] end=[Outputs] max-height=300px overflow-y=scroll label=False

Next, the sub-application input file must be created, which is shown below. This input file mimics the master input
file closely, with three notable exceptions. First, the [Kernels](/Kernels/index.md) block utilize the time
derivative and a new object, [LevelSetOlssonReinitialization](level_set/LevelSetOlssonReinitialization.md), that
implements the reinitialization scheme of \cite{olsson2007conservative}. Second, the [Problem](/Problem/index.md)
is set to use the [LevelSetReinitializationProblem](level_set/LevelSetReinitializationProblem.md). Finally, the
[UserObjects](/UserObjects/index.md) block includes a terminator, [LevelSetOlssonTerminator](level_set/LevelSetOlssonTerminator.md), which is responsible for stopping the reinitialization
solve when steady-state is achieved according to the criteria defined by \citet{olsson2007conservative}.

!text modules/level_set/examples/rotating_circle/circle_rotate_sub.i max-height=300px overflow-y=scroll

Figure \ref{circle_rotate_master_out} shows the results of the bubble problem with reinitialization, the result looks
similar to the SUPG result. However, if you consider the area conservation discussed in the [Area Comparison](#area_comparison) section, the reinitialization scheme yields the superior solution for this problem.

!figure docs/media/level_set/circle_rotate_out.gif id=circle_rotate_out width=32% margin-right=2% float=left caption=Results from solving the rotating circle problem with the level set equation alone.

!figure docs/media/level_set/circle_rotate_supg_out.gif id=circle_rotate_supg_out width=32% margin-right=2% float=left caption=Results from solving the rotating circle problem with the level set equation using SUPG stabilization.

!figure docs/media/level_set/circle_rotate_master_out.gif id=circle_rotate_master_out width=32% float=left caption=Results from solving the rotating circle problem with the level set equation with reinitialization.

## Area Comparison

Figure \ref{area_comparison} is a plot of the area of
the circle during the three simulations. Note that in the
unstabilized, un-reinitialized level set equation, both area
conservation and stability issues are readily apparent. The
instabilities are especially obvious in Figure \ref{area_comparison}, where the drastic
area changes are due to numerical oscillations in the solution
field. Adding SUPG stabilization helps ameliorate the stability
concern but it suffers from loss of area conservation. The
re-initialization scheme is both stable and area-conserving.

!figure docs/media/level_set/area_comparison.png id=area_comparison caption=Comparison of area inside the bubble during simulations. width=50% float=right margin-left=20px

The re-initialization methods performs well, but it is computationally
expensive and picking the pseudo timestep size $\Delta \tau$, steady-state
criteria $\delta$, and
interface thickness $\epsilon$ correctly for the re-initialization
problem is a non-trivial difficulty. Nevertheless, the level set
module provides a valuable starting point and proof-of-concept for
other researchers interested in the method, and the existing algorithm
can no doubt be tuned to the needs of specific applications in terms
of conservation and computational cost requirements.

## References
\bibliographystyle{unsrt}
\bibliography{docs/bib/level_set.bib}
