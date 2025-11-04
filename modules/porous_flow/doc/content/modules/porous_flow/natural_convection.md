# Natural Convection

## Problem statement

This example is based on the Elder problem [!cite](Elder1967TransientCI), see [!citet](Oldenburg1995OnTD). The natural convection of water in a 2D porous tank heated on one part of the bottom is modelled. The schematic is demonstrated in [schematicFig]. The experiment details are elaborated in the referred paper so will not be shown here to avoid repetition. The governing equations and associated `Kernels` that describe this problem are provided in detail [here](porous_flow/governing_equations.md).


!table id=datatable caption=Parameters of the problem [!citep](Oldenburg1995OnTD).
| Symbol | Quantity | Value | Units |
| --- | --- | --- | --- |
| $\phi$ | Porosity | $0.1$ | $-$ |
| $k$ | Permeability | $1.21*10^{-10}$  | $m^2$ |
| $\mu$ | Viscosity ($T=20^oC$) | $1*10^{-3}$  | $J.Kg^{-1}.s^{-1}.m^{-1}$ |
| $K$ | Thermal conductivity | $1.49$  | $J.Kg^{-1}.^oC^{-1}$ |
| $C$ | Heat capacity of rock | $0$ | $m^2$ ||
| $g$ | Gravity | $9.81$  | $m.s^{2}$ |
| $T_0$ | Initial temperature | $12$  | $^oC$ |

!media media/porous_flow/Convec_schematic.png style=width:100%;margin-left:10px; caption=The experiment schematic [!citep](Oldenburg1995OnTD). id=schematicFig


## Input file setup

This section describes the input file syntax.

### Mesh

The first step is to set up the mesh. In this problem, a rectangular tank with a width of 300 m and a height of 150 m is simulated. For convenience, instead of the YZ coordinate as shown in [schematicFig], the simulation was set up in XY coordinate with the positive Y direction pointing toward the top of the tank. In this code block, the numbers of nodes in the X and Y directions are specified along with the dimension of the tank. A notable feature is the declaration of the boundary `heater`. This is due to the fact that the heat element at the bottom of the tank is located between 0< x < 150 m. This enables simple boundary condition implementation at later stages.

!listing modules/porous_flow/examples/natural_convection/natural_convection.i block=Mesh

### Fluid properties

After the mesh has been specified, we need to provide the fluid properties that are used for this simulation. For our case, it will be just water and can be easily implemented as follows:

!listing modules/porous_flow/examples/natural_convection/natural_convection.i block=FluidProperties

### Variables declaration

Since we are focusing on the natural convection of flow in a porous medium, temperature (T) and porepressure are used.

!listing modules/porous_flow/examples/natural_convection/natural_convection.i block=Variables

### Action declaration

For this problem, we use the [`PorousFlowFullySaturated`](PorousFlowFullySaturated.md) to automatically add all of the physics kernels and a number of the necessary material properties. As this is a TH problem, we set `coupling_type = Thermohydro`.

!listing modules/porous_flow/examples/natural_convection/natural_convection.i block=PorousFlowFullySaturated

Additional material properties not added by the action are specified in the `Materials` block.

!listing modules/porous_flow/examples/natural_convection/natural_convection.i block=Materials

### Initial and Boundary Conditions

The next step is to supply the initial and boundary conditions. For this problem, two initial conditions need to be implemented. The first one is a constant temperature of 285 K which was declared in the previous section. The second one is the hydrostatic variation of porepressure along the y direction as shown below:

!listing modules/porous_flow/examples/natural_convection/natural_convection.i block=ICs

Shifting to the boundary conditions. There are three Dirichlet boundary conditions that we need to supply. The first two are constant temperature at the top and heater surfaces denoted as `t_top` and `t_bot`. And, the last one is constant pressure at the top surface denoted as `p_top`.

!listing modules/porous_flow/examples/natural_convection/natural_convection.i block=BCs

### Executioner setup

Lastly, we will tell Moose the type of the problem (shown in the code block below). For this problem, a transient solver is required.
To save time and computational resources, [IterationAdaptiveDT](IterationAdaptiveDT.md) and mesh [Adaptivity](Adaptivity.md)
were implemented. The former enables the time step to be increased if the solution converges and decreased if it does not. This will help save time if a large time step can be used and aid in convergence if a small time step is required. Similarly, the latter enables the mesh to automatically adapt to optimize the computational resource usage and resolve the area with stiff gradients. The mesh will be coarsened at the regions with low error and refined at the critical areas (high error and stiff gradients). For practice, users can try to disable them by putting the code block into comment and witnessing the difference in the solving time and solution.

!listing modules/porous_flow/examples/natural_convection/natural_convection.i block=Executioner

### Auxiliary kernels and variables (optional)

This section is optional since these kernels and variables do not affect the calculation of the desired variable. In this example, we want to output the fluid density at the end of the simulation thus we will add an auxkernel and an auxvariable for it. It can be noticed that the term `execute_on = TIMESTEP_END` indicates this kernel will be activated to calculate the fluid density at the end of each time step only, thus this calculation does not affect the performance of other tasks.

!listing modules/porous_flow/examples/natural_convection/natural_convection.i block=AuxVariables

!listing modules/porous_flow/examples/natural_convection/natural_convection.i block=AuxKernels

## Result

The input file can be executed and the result is as follows:

!media media/porous_flow/natural_convection_video.mp4 style=width:100%;margin-left:10px; caption=Spatial distribution of temperature with a test time of 2 years. id=Temp_twoyears_vid

It is obvious that the mesh at the interface between hot and cold fluid is much more refined as discussed while surrounding areas have a coarser mesh.

!media media/porous_flow/natural_convection_last_frame.png style=width:100%;margin-left:10px; caption=Spatial distribution of temperature at the end of 2 years with mesh grid displayed. id=Temp_twoyears_lastframe

The result from the simulation agrees well with published literature.

!media media/porous_flow/Convec_result.png style=width:100%;margin-left:10px; caption=Temperature contours and velocity vectors after 2 years [!citep](Oldenburg1995OnTD). id=past_result


!bibtex bibliography
