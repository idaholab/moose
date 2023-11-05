# Multiple Load SIMP

We employ here a multi-app scheme similar to that employed in
[Thermal and mechanical optimization](topology_optimization/thermomechanical.md).
In this example, each subapp is solving the same mechanics problem with a different applied load.
The sensitivity of the structure to each load is computed separately and used in a
global optimization algorithm. Considering the optimization of a problem with
the separate effect of the loads, instead
of accounting for its simultaneous application, can yield a final optimized system whose
material distribution is a significantly different from a simple concomitant approach
(see [!cite](bendsoe2003topology)). In the extreme case,
having two loads acting on the same point in opposite directions would yield an ill-defined
optimized structure if such loads are considered to be concomitant. However, the independent
combination of each of the load's sensitivities would yield a structure that is optimized to
reduce the compliance under the desired load sensitivity weighting.

In this example, we consider a bridge-like structure on which two vertical loads are applied.
The multiapp approach sends the updated pseudo-densities to the subapps, whereas said subapps
send their parent app the new load sensitivities. The subapp input files are almost identical,
with different load application points:

!listing examples/optimization/multi-load/single_subapp_one.i
         block=UserObjects id=bc_var_block_a
         caption=MBB Application point one

!listing examples/optimization/multi-load/single_subapp_two.i
         block=UserObjects id=bc_var_block_b
         caption=MBB Application point two

Filtering of sensitivities takes place in the subapps with the main application being responsible for
performing the optimization process.

!listing examples/optimization/multi-load/single_main.i
         block=UserObjects id=bc_var_block_c
         caption=MBB Density update in main application


If considered to act simultaneously, the optimized structure becomes an arch. If, on the
other hand, the structural responses to each of loads are combined, a more robust
triangular frame resurfaces as an optimized design. See figure below for one solution to
multi-load problem:

!media large_media/optimization/multiload.png style=width:75%



