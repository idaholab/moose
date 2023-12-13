# Mixing of CO2 and CH4 gases in a porous media

## Problem statement

This example is based on [!cite](PRUESS20041431). The mixing between $CO_2$ and $CH_4$ in a porous media is modelled. The schematic is demonstrated in [!ref](schematic_1D_mix). The experiment details are elaborated in the referred paper so will not be shown here to avoid repetition. The parameters used in this problem are provided in [!ref](datatable). The governing equations and associated `Kernels` that describe this problem are provided in detail [here](governing_equations.md).


!table id=datatable caption=Parameters of the problem.
| Symbol | Quantity | Value | Units |
| --- | --- | --- | --- |
| $\phi$ | Porosity | $0.1$ | $-$ |
| $k$ | Permeability | $1*10^{-14}$  | $m^2$ |
| $d$ | Molecular diffusion | $1*10^{-7}$   | $m^2/s$ |
| $T_0$ | Initial temperature | $313.15$  | $^oK$ |
| $t$ | Total time | $100$  | years |


!media porous_flow/CH4_CO2_1D_vertical.png caption=The schematic of the problem.  id=schematic_1D_mix

## Input file setup

This section describes the input file syntax.

### Mesh

The first step is to set up the mesh. In this problem, a  tank with a length of 100 m and a width of 10 m is simulated and an element size of 1 m is used. Additionally, two blocks `top_box`, and `bot_box` are defined for later initial conditions implementation.

!listing modules/porous_flow/examples/CH4_CO2/CH4_CO2_vertical.i block=Mesh

### Fluid properties

After the mesh has been specified, we need to provide the fluid properties that are used for this simulation. For our case, they will be carbon dioxide ($CO_2$) and methane ($CH_4$) and can be easily implemented as shown below.

!listing modules/porous_flow/examples/CH4_CO2/CH4_CO2_vertical.i  block=FluidProperties

### Variables declaration

Then, we need to declare what type of unknown we are solving. This can be done in the variable code block. Since we are focusing on the variation of the mass fraction of each gas, mass_frac_CH4 is a desired variable. pp_CO2 is also used for the fluid motion.

!listing modules/porous_flow/examples/CH4_CO2/CH4_CO2_vertical.i block=Variables

### Kernel declaration

This section describes the physics we need to solve. To do so, some kernels are declared. In MOOSE, the required kernels depend on the terms in the governing equations. For this problem, six kernels were declared. The code block is shown below:

!listing modules/porous_flow/examples/CH4_CO2/CH4_CO2_vertical.i block=Kernels


### Material setup

Additional material properties are required, which are declared here:

!listing modules/porous_flow/examples/CH4_CO2/CH4_CO2_vertical.i block=Materials

### Boundary Conditions

The next step is to supply the boundary conditions. For this problem, since all boundaries are wall, no boundary condition needs to be supplied.

!listing modules/porous_flow/examples/CH4_CO2/CH4_CO2_vertical.i block=BCs

### Initial Conditions

Then we need to supply the initial conditions. For this problem, the initial mass fraction of methane is set to 1 at the bottom half of the tank and 0 at the top half. Additionally, a constant pressure condition is supplied. The code is provided below:

!listing modules/porous_flow/examples/CH4_CO2/CH4_CO2_vertical.i block=ICs

### Executioner setup

For this problem, a transient solver is required.
To save time and computational resources, [`IterationAdaptiveDT`](IterationAdaptiveDT.md) was implemented. This option enables the time step to be increased if the solution converges and decreased if it does not. Thus, this will help save time if a large time step can be used and aid in convergence if a small time step is required. For practice, users can try to disable it by putting the code block into comment and witnessing the difference in the solving time and solution.

!listing modules/porous_flow/examples/CH4_CO2/CH4_CO2_vertical.i block=Executioner

## Result

The simulated mixing process is shown in [!ref](mixing_video), it can be seen that the density is gradually smoothened over time and the mixing occurs with the highest rate in the middle due to the stiff density gradient. The $CO_2$ molar fraction distribution is provided in [!ref](Result_1D) and it shows a strong agreement with other software as shown in [!ref](benchmark). This highlights MOOSE's capability of accurately simulating the density-driven diffusive mixing of multiple gases.


!media porous_flow/CH4_CO2.mp4 caption= The mixing process of $CO_2$ and $CH_4$.  id=mixing_video

!media porous_flow/CO2_molar_frac.png caption= $CO_2$ molar fraction distribution at different times.  id=Result_1D

!media porous_flow/CO2_molar_frac_benchmark.png caption= The benchmark conducted by [!cite](PRUESS20041431).  id=benchmark

