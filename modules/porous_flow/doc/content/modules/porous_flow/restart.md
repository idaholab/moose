# Restarting and recovering from previous simulations

MOOSE provides several options for [restarting or recovering](application_usage/restart_recover.md optional=True) from previous simulations. In this example, we demonstrate how this capability can be used in the PorousFlow module by considering a simple two-part problem. The first is to establish the hydrostatic pressure gradient in a reservoir that is due to gravity. This is the equilibrium state of the reservoir, and will be used as the initial condition for porepressure in the second part of this problem, where gas is injected into the reservoir in a single injection well.

Although this problem is simple, it demonstrates how the results of a previous simulation can be used to provide the initial condition for a second simulation.

## Model

The properties of the reservoir used in the model are summarized in [tab:res]. The pressure gradient is
representative of a reservoir at a depth of approximately 1,000 m.

!table id=tab:res caption=Reservoir properties
| Property |  Value |
| - | - |
| Height | 100 m |
| Length | 5,000 m |
| Pressure | Hydrostatic (10 - 11 MPa) |
| Temperature | 50 $^{\circ}$C |
| Permeability | $10^{-13}$ m$^2$ (~100 md) |
| Porosity | 0.1 |
| NaCl mass fraction | 0.1 |
| Methane injection rate | 1 kg/s |

Relative permeability of the aqueous phase is represented by a [Corey](/PorousFlowRelativePermeabilityCorey.md) model, with exponent $n = 2$ and residual saturation
$S_{r} = 0.2$. Gas phase relative permeability is also represented using a [Corey](/PorousFlowRelativePermeabilityCorey.md) model, with exponent $n = 2$ and residual saturation $S_r = 0.1$. Capillary pressure is given by a [van Genuchten](/PorousFlowCapillaryPressureVG.md) model, with exponent $m = 0.5$, and residual saturation $S_{r} = 0.2$.


## Gravity equilibrium

The hydrostatic pressure gradient in the reservoir is the result of gravity and the density of the resident brine. This pressure gradient can be calculated using a fully saturated single-phase model with a relatively coarse mesh. As this pressure gradient depends on gravity, it develops in the vertical direction only, and can therefore be modelled using a two-dimensional Cartesian mesh.

The following input file to establish hydrostatic equilibrium in this model is provided:

!listing modules/porous_flow/examples/restart/gravityeq.i

In this example, an initial guess at the hydrostatic equilibrium is provided using a function with an estimate of the brine density at the given pressure and temperature (approximately 1060 kg/m$^3$)

!listing modules/porous_flow/examples/restart/gravityeq.i block=Functions

The simulation is set to run for approximately 100 years, with steady-state detection enabled to end the run when a steady-state solution has been found

!listing modules/porous_flow/examples/restart/gravityeq.i block=Executioner

This input file quickly establishes gravity equilibrium in twelve time steps, with a hydrostatic pressure gradient shown in [fig:hydrostatic_pressure].

!media media/porous_flow/hydrostatic_pressure.png
       id=fig:hydrostatic_pressure
       style=width:80%;margin-left:10px;
       caption=Hydrostatic pressure gradient at gravity equilibrium

## Basic restart

The results of the gravity equilibrium simulation can be used as initial conditions for a more complicated simulation. In this case, we now consider injection of methane through an injection well in a two-dimensional radial model constructed by rotating the mesh used in the gravity equilibrium run around the Y-axis.

In contrast to the simple gravity equilibrium model, we now use a two-phase model, with the initial condition for the liquid porepressure taken from the final results of the gravity equilibrium simulation.

As the mesh is identical in both cases (despite the rotation about the Y-axis), we can use the simple variable initialisation available in MOOSE.

The input file for this case is

!listing modules/porous_flow/examples/restart/gas_injection.i

The input mesh for this problem is the output mesh from the gravity equilibrium simulation, which we uniformly refine once to increase the spatial resolution, translate horizontally and rotate about the Y-axis to form a two-dimensional mesh with an injection well at the centre

!listing modules/porous_flow/examples/restart/gas_injection.i block=Mesh Problem

The initial condition for the liquid porepressure is read directly from the input mesh

!listing modules/porous_flow/examples/restart/gas_injection.i block=Variables

Running this model for a total of 10$^8$ s, we obtain the gas saturation profile shown in [fig:gas_injection].

!media media/porous_flow/gas_injection.png
       id=fig:gas_injection
       style=width:80%;margin-left:10px;
       caption=Gas saturation after 10$^8$ seconds

Due to the large density contrast between the injection methane (with a density of 175 kg m$^3$ compared to approximately 1060 kg m$^3$), the methane migrates vertically and pools beneath the top of the reservoir, extending to a radial distance of approximately 900 m. Methane immobilised at residual saturation can be observed in the lower part of the reservoir (where the gas saturation is approximately 0.1).

## Flexible restart using a SolutionUserObject

In the above example, the mesh used in the gas injection problem is initially identical to the mesh used in the gravity equilibrium run. MOOSE provides a more flexible restart capability where the mesh used in the subsequent simulation does not have to be identical to the mesh used in the initial simulation. Instead, the variable values from the initial simulation can be projected onto a new mesh in a subsequent simulation using a [SolutionUserObject](/SolutionUserObject.md) and
[SolutionFunction](/SolutionFunction.md).

It is clear from the results shown above that the methane is expected near the upper left of the model (near the top of the injection well). We could refine the original mesh in that region using  mesh adaptivity, but we instead construct a new mesh that is refined in that region and use a [SolutionUserObject](/SolutionUserObject.md) and [SolutionFunction](/SolutionFunction.md) to project the initial value for liquid porepressure from the coarse gravity equilibrium mesh to the new mesh.

The input file for this example is

!listing modules/porous_flow/examples/restart/gas_injection_new_mesh.i

The mesh in this example is biased so that there are more elements in the upper left region, and fewer away from the well

!listing modules/porous_flow/examples/restart/gas_injection_new_mesh.i block=Mesh

The results from the gravity equilibrium simulation are read using a SolutionUserObject

!listing modules/porous_flow/examples/restart/gas_injection_new_mesh.i block=UserObjects/soln

A SolutionFunction is used to spatially interpolate these values

!listing modules/porous_flow/examples/restart/gas_injection_new_mesh.i block=Functions/ppliq_ic

and finally a [FunctionIC](/FunctionIC.md) is used to set the initial condition of the liquid porepressure

!listing modules/porous_flow/examples/restart/gas_injection_new_mesh.i block=ICs

This more refined model can then be run, resulting in the gas saturation profile shown in [fig:gas_injection_refined].

!media media/porous_flow/gas_injection_refined.png
       id=fig:gas_injection_refined
       style=width:80%;margin-left:10px;
       caption=Gas saturation after 10$^8$ seconds

Due to the additional refinement near the well and the top of the reservoir, the gas saturation profile is now smoother near the well, with much less lateral migration in the lower part of the reservoir and hence much less methane immobilised at residual saturation. As a result, more methane is able to migrate to the top of the reservoir, and the lateral plume extends nearly 200 m further in this model than the coarser model shown above.

## Generalisation to more complicated models

The above example of the restart capability demonstrates how PorousFlow can use the results of previous simulations as initial conditions of new simulations, even when changing from single-phase models to a more complex two-phase model. Of course, these approaches can be extended to models where heat and geomechanics are involved by setting initial conditions for additional variables using an identical approach to that demonstrated in this simple example.

## Recovery

A simulation may fail prematurely due to external factors (such as exceeding the wall time on a cluster). In these cases a simulation may be recovered provided that checkpointing has been enabled.

!listing modules/porous_flow/examples/restart/gas_injection.i block=Outputs

If the simulation `gas_injection.i` is interrupted for any reason, it may be recovered using the checkpoint data

```language=bash
porous_flow-opt -i gas_injection.i --recover
```

whereby the simulation will continue from the latest checkpoint data.

For this reason, it is strongly recommended that checkpointing is enabled in long-running jobs.
