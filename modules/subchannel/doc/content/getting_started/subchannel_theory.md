# SubChannel Theory

## Introduction

&nbsp;

The diversity of the reactor Gen-IV designs proposed by the industry in recent years, necessitates maintenance and support (M\&S) software that permits flexible multi-physics capabilities. [MOOSE] [!cite](GASTON2009) is a Multi-physics Object Oriented Simulation Environment, a parallel computational framework targeted at the solution of coupled, nonlinear partial differential equations (PDEs) that often arise in simulation of nuclear processes. The main advantage of the MOOSE framework is that its a flexible finite element tool that leverages the multi-physics capabilities inherent in MOOSE. Gen-IV reactors present a significant challenge in their analysis due to their complexity, advanced performance and new design features. Developing novel nuclear reactor designs such as the Gen-IV and ensuring their safety under normal operating conditions, operational transients, anticipated operational occurrences, design basis accidents (DBA) etc. required the development of novel computational tools. The advances in available computational power and novel computational methods, have enabled engineers to develop the those tools. These codes solve the various physics related to nuclear reactors. Neutronics, fuel performance, and thermal-hydraulics, form the primary set of physics that needs to be resolved.

Subchannel codes are thermal-hydraulic codes that offer an efficient compromise for the simulation of a nuclear reactor core, between CFD and system codes. They use a quasi-3D model formulation and allow for a finer mesh than system codes without the high computational cost of CFD. That's why thermal-hydraulic  analysis of  a nuclear reactor core is mainly performed using the subchannel type of codes to estimate the various thermal-hydraulic safety margins and the various quantities of interest. The safety margins and the operating power limits of the nuclear reactor core under different conditions, i.e., system pressure, coolant inlet temperature, coolant flow rate, thermal power, and their distributions are considered as the key parameters for subchannel analysis [!cite](SHA1980).

## Governing Equations


The subchannel thermal-hydraulic analysis is based on the conservation equations of mass, linear momentum and energy on the specified control volumes. The control volumes are connected in both axial and radial directions to get the three dimensional effect of the reactor core. The subchannel control volumes are shown in [ControlVolume] from [!cite](Todreas).

!media figures/ControlVolume.png
    style=width:90%;margin-bottom:2%;margin:auto;
    id=ControlVolume
    caption=Square Lattice subchannel control volume

The subchannel equations are derived by integrating and averaging the conservation equations over the subchannel control volumes.

### Mass conservation equation
\begin{equation}
\label{mass-conservation-equation}
\frac{d\rho_i}{dt} V_i +\Delta \dot{m_i}+\sum_{j} w_{ij} = 0,
\end{equation}

where *i* is the subchannel index and *j* the index of the neighbor subchannel. $\Delta$ refers to the difference between the inlet and outlet of the control volume in the axial direction. $\dot{m_i}[kg/sec]$ is the mass flow rate of subchannel *i* in the axial direction.  $w_{ij}[kg/sec]$ is the diversion crossflow in the lateral direction from subchannel *i* to neighboring subchannel *j*, resulting from local pressure differences between the two subchannels.

### Axial momentum conservation equation
\begin{equation}
\label{conservation-axial-momentum}
\frac{d\dot{m_i}}{dt}\Delta Z+ \Delta(\frac{\dot{m_i}^2}{S_i \rho_i}) + \sum_{j}w_{ij} U^\star =
-S_i \Delta P_i+ Friction_i + Drag_{ij} - g  \rho_i  S_i \Delta Z
\end{equation}

In adition to the temporal term in the left hand side there is the change of momentum in the axial direction $\Delta(\frac{\dot{m_i}^2}{S_i^z \rho_i})$ and the inertia transfer between subchannels due to diversion crossflow $\sum_{j}w_{ij} U^\star$. $U^*$ is the axial velocity of the donor cell and $- g \rho_i S_i \Delta z$ represents the gravity force, where $g$ is the acceleration of gravity. It is assumed that gravity is the only significant body force in the axial momentum equation. The donor cell is the cell from which crossflow flows out of and depends on the sign of $w_{ij}$. If it is positive, the donor cell is*i*and if it is negative, the donor cell is*j*. Henceforward donor cell quantities will be denoted with the star ($^*$) symbol. $Friction_i$ is caused by fluid/rod interface and may also include possible local form loss due to spacers/mixing-vanes. $Drag_{ij}$ is caused by viscous stresses at the interface between subchannels.

### Lateral momentum conservation equation
\begin{equation}
\label{lateral-momentum}
\frac{dw_{ij}}{dt} L_{ij} + \frac{L_{ij}}{\Delta Z} \Delta (w_{ij} \bar{U}) = - S_{ij}  \Delta P_{ij} + Friction_{ij}
\end{equation}

Here $g_{ij}$ is the gap between subchannels *i,j* and $\Delta Z$ the height of the control volume. Lateral pressure gradient ($\Delta P_{ij} / L_{ij}$) across the subchannels and/or forced mixing between subchannels owing to mixing vanes and spacer grids is the driving force behind diversion crossflow $W_{ij}$. $L_{ij}$ is the distance between the centers of subchannels *i,j*. $\bar{U_{ij}}$ is the average axial velocity of the two subchannels. The overall friction loss term $Friction_{ij}$ encompasses all the viscous effects and form losses associated with momentum exchange between the fluid and the wall due to the fluid motion through the gap.

### Enthalpy conservation equation
\begin{equation}
\label{enthalpy-conservation}
\frac{d\left\langle \rho h\right\rangle_i }{dt}V_i + \Delta (\dot{m_i} h_i)  + \sum_{j} w_{ij} h^\star  + h'_{ij} = q'_i \Delta Z
\end{equation}

For a single-phase fluid, dissipation due to viscous stresses can be neglected and the total derivative of pressure (work of pressure) set to zero. Also there is no volumetric heat source due to moderation since heat is mainly transferred to the fluid through the fuel rods surface. $h'_{ij}$ is the turbulent enthalpy transfer between subchannels *i,j* and $q'_i$ is the average linear power $[\frac{kW}{m}]$ going into the control volumes of subchannel *i* from the fuel rods.

## Closure Models

#### Axial direction friction term
\begin{equation}
Friction_i = -\frac{1}{2} K_i \frac{\dot{m_i} |\dot{m_i}|}{S_{i} \rho_i }
\end{equation}

where $K_{i} = [\frac{f_w}{Dhy_i} \Delta Z + k_i]$ is an overall axial loss coefficient encompassing local concentrated form losses $k_i$ due to the changing of the flow area or due to the narrowing of the surface area and frictional losses $\frac{f_w}{Dhy_i} \Delta Z$ due to fluid/rod interaction. $S_{i}$ is the axial flow area, $f_w = 4f$ is the Darcy friction factor and $Dhy_i = \frac{4 S_i}{P_w}$ is the hydraulic diameter.

#### Lateral direction friction term
\begin{equation}
Friction_{ij}  =  -\frac{1}{2} g_{ij} \Delta Z K_{ij} \rho_{} |u_{ij}| u_{ij} = - \frac{1}{2}K_{ij} \frac{w_{ij}|w_{ij}|}{S_{ij} \rho^\star}.
\end{equation}

where $K_{ij}$ is an overall loss coefficient encompassing lateral concentrated form and friction losses and $S_{ij}$ the lateral flow area between subchannel *i* and subchannel *j*: $S_{ij} = \Delta Z g_{ij}$, $\rho^*$ is the donor cell density.

#### Friction factor [!cite](KIT)
\begin{equation}
f_w \rightarrow
\begin{cases}
\frac{1}{64} , & Re < 1\\
\frac{64}{Re}, &1 \leq Re<5000\\
0.316 Re^{-0.25}, &5000 \leq Re < 30000\\
0.184 Re^{-0.20}, &30000 \leq Re
\end{cases}
\end{equation}

#### Turbulent momentum diffusion
\begin{equation}
Drag_{ij} = -C_{T}\sum_{j} w'_{ij}\Delta U_{ij } = -C_{T}\sum_{j} w'_{ij}\big[ \frac{\dot{m_i}}{\rho_iS_i} - \frac{\dot{m_j}}{\rho_j S_j}\big].
\end{equation}

where $C_{T}$ is a turbulent modeling parameter.

#### Turbulent enthalpy diffussion
\begin{equation}
h'_{ij} = \sum_{j} w'_{ij}\Delta h_{ij} = \sum_{j} w'_{ij}\big[ h_i - h_j  \big].
\end{equation}

#### Turbulent crossflow
\begin{equation}
w'_{ij} = \beta S_{ij} \bar{G}, ~\frac{dw'_{ij}}{dz} = \frac{w'_{ij}}{\Delta Z}=\beta g_{ij} \bar{G}.
\end{equation}

where $\beta$ is the turbulent mixing parameter or thermal diffusion coefficient and $\bar{G}$ is the average mass flux of the adjacent subchannels. The $\beta$ term is the tuning parameter for the mixing model. Physically, it is a non-dimensional coefficient that represents the ratio of the lateral mass flux due to mixing to the axial mass flux. It is used to model the effect of the unresolved scales of motion that are produced through the averaging process. In single-phase flow no net mass exchange occurs, both momentum and energy are exchanged between subchannels, and their rates of exchange are characterized in terms of hypothetical turbulent interchange flow rates ($w_{ij}^{'H},w_{ij}^{'M}$) [!cite](TODREAS), for enthalpy and momentum respectively. The approximation that the rate of turbulent exchange for energy and momentum are equal is adopted: $w'_{ij} = w_{ij}^{'H} = w_{ij}^{'M}$.

## Algorithm

A hybrid numerical method of solving the subchannel equations was developed. Hybrid in this context means that the user has the option of solving portion of the problem at a time, by dividing the domain into blocks. Each block is solved sequentially from inlet to outlet. The mass flow at the outlet of the previous block and the pressure at the inlet of the next block provide the needed boundary conditions. The essense of the algorithm hinges on the construction of a combined residual function based on the lateral momentum equation. To solve this equation a Jacobian Free Newton-Krylov type Method (JFNKM) was used. The workhorse of the code is the non linear equation solvers (SNES) found in the Portable, Extensible Toolkit for Scientific Computation [PETSc](https://petsc.org/release/).

\begin{equation}
\label{lateral1}
f(w_{ij}) = \frac{dw_{ij}}{dt} L_{ij} + \frac{L_{ij} }{\Delta z} \Delta (w_{ij} \bar{U })  - S_{ij} \Delta P_{ij} + \frac{1}{2} K_{ij}
\frac{w_{ij}|w_{ij}|}{\rho^*}= 0.
\end{equation}

The main unknown variable in this non linear residual is the crossflow $w_{ij}$. The combined residual function calculates the non linear residual $f(w_{ij})$ after it updates the other main flow variables, such as mass flow $\dot{m}_i$,  turbulent crossflow $w'_{ij}$, pressure drop  $\Delta P_i$ and pressure $P_i$, using the current $w_{ij}$ as needed. So every time this function is called by the Newton solver the flow variables get updated. This affords the solution of all flow variables at the same time. $P_i$ is the local pressure minus the exit pressure, $P_i (z) - P_{exit}$, so at the exit $P_{i}$ is zero. The hybrid algorithm is presented in [stencil].

!media figures/stencil.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=stencil
    caption=Subchannel hybrid numerical scheme

Once the main flow variables converge in a block, the enthalpy conservation equation is solved and enthalpy $(h)$ is retrieved in all the nodes of the block. In the case where no heat is added to the fluid, the Enthalpy does not need to be calculated (unless there is a non-uniform enthalpy inlet distribution). Using enthalpy, pressure and the equations of state, temperature $T_i$ and the fluid properties such as density $\rho_i$ and viscosity $\mu_i$ are calculated. After the fluid properties are updated, the solve is repeated until the temperature converges. Once the temperature solution converges the procedure is repeated for the next block downstream. Once the temperature solution converges in all blocks we check to see if pressure has converged in all blocks. If not, we repeat the procedure starting again from the first block, until pressure has converged. Note that in order for the pressure information from the outlet to reach the inlet, it will require a number of the pressure loop iterations equal to the number of blocks. Last, the calculation of the flow variables and of the residual is done in an explicit manner.

### Algorithm variations

There are two variations [!cite](kyriakopoulos2022development), [!cite](kyriakopoulos2023demonstration) of the algorithm presented above:

#### Implicit segregated

In this case the conservation equations are recasted in Matrix form and the calculation of the flow variables and the residual is done in an implicit manner by solving each equation seperately.

#### Implicit

In this case the conservation equations are recasted in Matrix form and combined all together in a big Matrix. The calculation of the flow variables is done in an implicit manner by solving all the equations at the same time. The solution scheme used is a fixed point iterartion instead of a Newton method as done in the other versions. The advantage of the implicit scheme is increased robustness and the ability to calculate re-circulation flows.
