# Flow and Solute Transport in Porous Media

## 1D solute transportation

### Problem statement

This example is based on [!cite](w14081310). The transportation of tracer in water in a 1D porous tank is modelled. The schematic is demonstrated in [!ref](schematic_tracer). The experiment details are elaborated in the referred paper so will not be shown here to avoid repetition. The governing equations are provided below:

\begin{equation}
\label{eq:fluid_equa}
\frac{\partial p}{\partial t} + \nabla\cdot{\mathbf
  v} = 0.
\end{equation}

\begin{equation}
\label{eq:tracer_equa}
\frac{\partial C}{\partial t} + \nabla\cdot({\mathbf
  v}C - {\mathbf
  D_\beta}\nabla C) = 0.
\end{equation}

 Notation used in these equations is summarised in the
[nomenclature](/porous_flow/nomenclature.md).


!table id=datatable caption=Parameters of the problem .
| Symbol | Quantity | Value | Units |
| --- | --- | --- | --- |
| $\phi$ | Porosity | $0.25$ | $-$ |
| $k$ | Permeability | $1*10^{-11}$  | $m^2$ |
| $\mu$ | Viscosity ($T=20^oC$) | $1*10^{-3}$  | $J.Kg^{-1}.s^{-1}.m^{-1}$ |
| $g$ | Gravity | $9.81$  | $m.s^{2}$ |
| $T_0$ | Initial temperature | $293$  | $^oK$ |
| $t$ | Total time | $200$  | days |


!media porous_flow/schematic_tracer.png caption=The schematic of the problem [!cite](w14081310).  id=schematic_tracer

### Input file setup

This section describes the input file syntax.

### Mesh

The first step is to set up the mesh. In this problem, a  tank with a length of 100 m is simulated and an element size of 1 m is used.

!listing modules/porous_flow/examples/solute_tracer_transport/solute_tracer_transport.i block=Mesh

### Fluid properties

After the mesh has been specified, we need to provide the fluid properties that are used for this simulation. For our case, it will be just water and can be easily implemented as follows:

!listing modules/porous_flow/examples/solute_tracer_transport/solute_tracer_transport.i block=FluidProperties

### Variables declaration

Then, we need to declare what type of unknown we are solving. This can be done in the variable code block. Since we are focusing on the concentration of tracer at x =50 m, C is a desired variable. Porepressure is also used for the fluid motion. Since we only have constant initial conditions so they can be directly supplied here.

!listing modules/porous_flow/examples/solute_tracer_transport/solute_tracer_transport.i block=Variables

### Kernel declaration

This section describes the physics we need to solve. To do so, some kernels are declared. In MOOSE, the required kernels depend on the terms in the governing equations. For this problem, six kernels were declared. To have a better understanding, users are recommended to visit [this page](porous_flow/governing_equations.md). The code block is shown below with the first three kernels associated with equation 1  and the remain associated with equation 2.

!listing modules/porous_flow/examples/solute_tracer_transport/solute_tracer_transport.i start=[Kernels] end=[FluidProperties]


### Material setup

Additional material properties are required , which are declared here:

!listing modules/porous_flow/examples/solute_tracer_transport/solute_tracer_transport.i block=Materials

### Boundary Conditions

The next step is to supply the boundary conditions. There are three Dirichlet boundary conditions that we need to supply. The first two are constant pressure at the inlet and outlet denoted as `constant_inlet_pressure` and `constant_outlet_porepressure`. Another one is constant tracer concentration at the inlet denoted as `inlet_tracer`. Finally, a PorousFlowOutflowBC boundary condition was supplied at the outlet to allow the tracer to freely move out of the domain.

!listing modules/porous_flow/examples/solute_tracer_transport/solute_tracer_transport.i block=BCs

### Executioner setup

For this problem, a transient solver is required.
To save time and computational resources, [`IterationAdaptiveDT`](IterationAdaptiveDT.md) was implemented. This option enables the time step to be increased if the solution converges and decreased if it does not. Thus, this will help save time if a large time step can be used and aid in convergence if a small time step is required. For practice, users can try to disable it by putting the code block into comment and witnessing the difference in the solving time and solution.

!listing modules/porous_flow/examples/solute_tracer_transport/solute_tracer_transport.i block=Executioner

### Auxiliary kernels and variables (optional)

This section is optional since these kernels and variables do not affect the calculation of the desired variable. In this example, we want to know the Darcy velocity in the x direction thus we will add an auxkernel and an auxvariable for it.

!listing modules/porous_flow/examples/solute_tracer_transport/solute_tracer_transport.i block=AuxVariables

!listing modules/porous_flow/examples/solute_tracer_transport/solute_tracer_transport.i block=AuxKernels

### Postprocessor

As previously discussed, we only want to know the variation of concentration (C) and x-direction Darcy velocity at x=50 m over time. So, we need to tell MOOSE to only write that information into the result file via this code block.

!listing modules/porous_flow/examples/solute_tracer_transport/solute_tracer_transport.i block=Postprocessors

### Result

The results obtained from MOOSE are compared against the analytical solution proposed by [!cite](Ogata1961ASO).

!media porous_flow/concen_time.png caption= Tracer concentration with respect to time.  id=concen_time

!media porous_flow/RMS_tracer.png caption= Root-mean-square error of MOOSE results.  id=RMS_tracer

Compared with the benchmark conducted by [!cite](w14081310), it can be seen that MOOSE is capable of delivering accurate results for this type of problem.

!media porous_flow/benchmark_tracer.png caption= The benchmark conducted by [!cite](w14081310).  id=benchmark_tracer

## 2D solute transportation

### Problem statement

This example is based on [!cite](w14081310). The transportation of tracer in water in a 2D porous tank is modelled. The schematic is demonstrated in [!ref](schematic_tracer_2D). The parameters are the same as the 1D problem as provided in [!ref](datatable).

!media porous_flow/schematic_tracer_2D.png caption=The schematic of the 2D problem [!cite](w14081310).  id=schematic_tracer_2D

### Input file setup

The input file for this problem is similar to the 1D case except for some changes made to account for the 2D mesh and the changes in the boundary conditions which are elaborated in the following sections. Other sections will be the same as the 1D case.

### Mesh

For this problem, to simulate a 2D rectangular aquifer  with a point source placed at the origin, the x-domain was chosen from -50 to 50 m and the y-domain from 0 to 50 m.

!listing modules/porous_flow/examples/solute_tracer_transport/solute_tracer_transport_2D.i block=Mesh

### DiracKernels set-up

Since our problem comprises a point source, [`PorousFlowSquarePulsePointSource`](PorousFlowSquarePulsePointSource.md) DiracKernels are required. These DiracKernels inject the water and tracer into the domain at a constant rate. As can be seen below, two DiracKernels are used. The first one supplies the water with a specified mass flux to the system and the second one the tracer.

!listing modules/porous_flow/examples/solute_tracer_transport/solute_tracer_transport_2D.i block=DiracKernels

### Boundary Conditions

The boundary conditions are similar to the 1D case except for the addition of several Dirichlet, and outflow conditions due to the extra faces of the 2D problem. The bottom face was not supplied with any boundary condition as it a line of symmetry (only half the model is simulated). Therefore, no-flow is allowed across this boundary.

!listing modules/porous_flow/examples/solute_tracer_transport/solute_tracer_transport_2D.i block=BCs

### Result

The results obtained from MOOSE are compared against the analytical solution proposed by [!cite](Schroth).

!media porous_flow/concen_time_2D.png caption= Tracer concentration with respect to time.  id=concen_time_2D

!media porous_flow/RMS_tracer_2D.png caption= Root-mean-square error of MOOSE results.  id=RMS_tracer_2D

Compared with the benchmark conducted by [!cite](w14081310), it can be seen that once again MOOSE is capable of delivering accurate results for this type of problem.

!media porous_flow/benchmark_tracer_2D.png caption= The benchmark conducted by [!cite](w14081310).  id=benchmark_tracer_2D
