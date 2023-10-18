# Thermomechanical SIMP Optimization with Multi-Apps

In this example, we setup two different physics problems defined in the same domain.
The first model is a mechanical small deformation problem with two loads using symmetric
boundary conditions. Such a model yields the typical SIMP "bridge-like" structure.
The problem is set up regularly with the mechanical compliance sensitivity computed
and adequately filtered.

!listing examples/optimization/thermomechanical/structural_sub.i
         block=UserObjects id=bc_var_block_a
         caption=MBB User objects in structural subapp

The second model is a thermal, heat conduction problem with one heat generation boundary
(on the left) with the rest of boundaries defined as insulating material; i.e. zero
Neumann boundary condition on temperature. Such a model often generates a dendritic
structure to maximize the reduction in the temperature gradients. In a way analogous to
the structural problem, the thermal compliance sensitivities are generated and filtered
in the subapp:

!listing examples/optimization/thermomechanical/thermal_sub.i
         block=UserObjects id=bc_var_block_b
         caption=MBB User objects in thermal subapp

Finally, the main app obtain the sensitivities from the subapps and sends the computed
pseudo-densities to the subapps. This process tends to reduce the objective function of both
problems. The extent to which the one particular subapp drives the optimization process
depends on the weights used to obtain a total or overall sensitivity, which is used in
the density update process.

!listing examples/optimization/thermomechanical/thermomechanical_main.i
         block=UserObjects id=bc_var_block_c
         caption=MBB Density update in main app

The result of the optimization, which heavily depends on the many simulation and optimization
parameters, is shown in what follows:

!media large_media/optimization/tm.png style=width:75%



