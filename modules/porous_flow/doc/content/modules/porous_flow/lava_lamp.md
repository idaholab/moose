# Density-driven convective mixing

PorousFlow may be used to study the convective mixing of fluids.  In this page we discuss the density-driven convective mixing of CO$_{2}$ into brine.  CO$_{2}$ exists initially in both the liquid and gas phase.  As the CO$_{2}$ diffuses into the brine, the gaseous CO$_{2}$ disappears and the brine's density increases, which drives convective mixing of the two fluids and accelerates the dissolution of the gas phase [!citep](emami). Input files for similar situations may be found [here](https://github.com/idaholab/moose/blob/master/modules/porous_flow/examples/lava_lamp).

!alert note
These examples should be run using multiple cores due to the computational expense. It is recommended
that the single-phase model is run using at least four cores, and the two-phase model with at least eight cores.

Because this problem involves the disappearance of the gaseous phase, the simulation utilises persistent primary variables and a vapor-liquid flash.

!listing modules/porous_flow/examples/lava_lamp/2phase_convection.i block=Variables

Here `pgas` is the gas-state pressure (the capillary pressure is zero, so this is also the brine porepressure) and `zi` is the total mass fraction of the CO$_{2}$ summed over the two phases.  In this input file, the salt content of the brine is fixed at 0.01:

!listing modules/porous_flow/examples/lava_lamp/2phase_convection.i start=[xnacl] end=[]

and the temperature at 45$^{\circ}$C:

!listing modules/porous_flow/examples/lava_lamp/2phase_convection.i start=[temperature] end=[brineco2]

The porepressure is initialised to approximate hydrostatic conditions, and the total mass fraction of CO$_{2}$ is zero except towards the top of the model domain where it is 0.2.  This corresponds to a gas saturation of 0.29 (the gas is almost all CO$_{2}$ and only about 0.2% brine) and the mass-fraction of CO$_{2}$ in the liquid phase is 0.048.  The porosity is slightly randomised by sampling from a uniform distribution between the minimum and maximum values specified in the [RandomIC](RandomIC.md) initial condition, and this promotes initiation of the beautiful "fingers" of CO$_{2}$-rich brine seen in the results. The random field seed can be changed in the input file, or on the command line using [command line overriding](command_line_usage.md optional=True). The porosity field is output in the Exodus file that is created, so
can be visualized in the MOOSE GUI [peacock](python/peacock.md optional=True) or in an external visualization package.

!listing modules/porous_flow/examples/lava_lamp/2phase_convection.i block=ICs

The equations of state for the liquid brine, the CO$_{2}$ and the liquid brine-CO$_{2}$ mixture are the high-precision versions supplied by the [fluid_properties module](fluid_properties/index.md):

!listing modules/porous_flow/examples/lava_lamp/2phase_convection.i block=FluidProperties

!listing modules/porous_flow/examples/lava_lamp/2phase_convection.i start=[fs] end=[]

!listing modules/porous_flow/examples/lava_lamp/2phase_convection.i start=[brineco2] end=[porosity]

Quantities of interest are tracked using `AuxVariables` populated using the following `AuxKernels`

!listing modules/porous_flow/examples/lava_lamp/2phase_convection.i block=AuxKernels

The usual 2-phase Kernels are used in this example with zero molecular dispersion (but the diffusion coefficient is nonzero)

!listing modules/porous_flow/examples/lava_lamp/2phase_convection.i block=Kernels

!listing modules/porous_flow/examples/lava_lamp/2phase_convection.i start=[diffusivity] end=[]

Various postprocessors help with tracking the total quantity of CO$_{2}$ in the two phases, and the flux of CO$_{2}$ into the brine:

!listing modules/porous_flow/examples/lava_lamp/2phase_convection.i block=Postprocessors

Mesh adaptivity is used to refine the mesh as the fingers form, based on the jump in the gradient of the total mass fraction of CO$_{2}$ summed over the two phases:

!listing modules/porous_flow/examples/lava_lamp/2phase_convection.i block=Adaptivity

The evolution of the CO$_{2}$ mass fraction is shown in the animation below

!media porous_flow/2phase_convection.gif caption=Left: Mass fraction of CO$_2$ in the liquid phase.  Right: Saturation of gas.  The adaptive mesh is overlayed on each figure.  id=2phase_convection_anim

!bibtex bibliography
