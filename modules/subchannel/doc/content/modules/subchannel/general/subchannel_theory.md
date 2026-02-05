# SubChannel Theory

## Introduction

&nbsp;

The diversity of the reactor Gen-IV designs necessitates design, maintenance and support (M&S) software that permits flexible multi-physics capabilities. MOOSE, the Multi-physics Object Oriented Simulation Environment, a parallel computational framework targeted at the solution of coupled, nonlinear partial differential equations (PDEs) that often arise in simulation of nuclear processes. The main advantage of the MOOSE framework is that its a flexible finite element and finite volumes tool in which multiple physics solvers can naturally be coupled. Gen-IV reactors present a significant challenge in their analysis due to their complexity, innovations, and new design features ensuring physics-based passive safety notably. Developing novel nuclear reactor designs and ensuring their safety under normal operating conditions, operational transients, anticipated operational occurrences, design basis accidents (DBA) etc. required the development of novel computational tools. These codes solve the various physics related to nuclear reactors. Neutronics, fuel performance, and thermal-hydraulics, form the primary set of physics that needs to be resolved.

Subchannel codes are thermal-hydraulic codes that offer an efficient compromise for the simulation of a nuclear reactor core, between CFD and system codes. They use a quasi-3D model formulation and allow for a finer mesh than system codes without the high computational cost of CFD. That's why thermal-hydraulic  analysis of  a nuclear reactor core is often performed using the subchannel type of codes to estimate the various thermal-hydraulic safety margins and the various quantities of interest. The safety margins and the operating power limits of the nuclear reactor core under different conditions, i.e., system pressure, coolant inlet temperature, coolant flow rate, thermal power, and their distributions are considered as the key parameters for subchannel analysis [!cite](SHA1980).

## Governing Equations

!! Intentional comment to provide extra spacing

The subchannel thermal-hydraulic analysis is based on the conservation equations of mass, linear momentum and energy on the specified control volumes. The control volumes are connected in both axial and radial directions to capture the three dimensional effects of the flow geometry. The subchannel control volumes are shown in [ControlVolume] from [!cite](todreas2021nuclear2).

!media subchannel/getting_started/ControlVolume.png
    style=width:90%;margin-bottom:2%;margin:auto;
    id=ControlVolume
    caption=Square Lattice subchannel control volume

The subchannel equations are derived by integrating and averaging the conservation equations over the subchannel control volumes.

### Mass conservation equation

!! Intentional comment to provide extra spacing

\begin{equation}
\label{mass-conservation-equation}
\frac{d\rho_i}{dt} V_i +\Delta \dot{m_i}+\sum_{j} w_{ij} = 0,
\end{equation}

where *i* is the subchannel index and *j* the index of the neighbor subchannel. $\Delta$ refers to the difference between the inlet and outlet of the control volume in the axial direction. $\dot{m_i}[kg/sec]$ is the mass flow rate of subchannel *i* in the axial direction.  $w_{ij}[kg/sec]$ is the diversion crossflow in the lateral direction from subchannel *i* to neighboring subchannel *j*, resulting from local pressure differences between the two subchannels.

### Axial momentum conservation equation

!! Intentional comment to provide extra spacing

\begin{equation}
\label{conservation-axial-momentum}
\frac{d\dot{m_i}}{dt}\Delta Z+ \Delta(\frac{\dot{m_i}^2}{S_i \rho_i}) + \sum_{j}w_{ij} U^\star =
-S_i \Delta P_i+ Friction_i + Drag_{ij} - g  \rho_i  S_i \Delta Z
\end{equation}

In addition to the temporal term in the left hand side there is the change of momentum in the axial direction $\Delta(\frac{\dot{m_i}^2}{S_i^z \rho_i})$ and the inertia transfer between subchannels due to diversion crossflow $\sum_{j}w_{ij} U^\star$. $U^*$ is the axial velocity of the donor cell and $- g \rho_i S_i \Delta z$ represents the gravity force, where $g$ is the acceleration of gravity. It is assumed that gravity is the only significant body force in the axial momentum equation. The donor cell is the cell from which crossflow flows out of and depends on the sign of $w_{ij}$. If it is positive, the donor cell is *i* and if it is negative, the donor cell is *j*. Henceforward donor cell quantities will be denoted with the star ($^*$) symbol. $Friction_i$ is caused by fluid/pin interface and may also include possible local form loss due to spacers/mixing-vanes. $Drag_{ij}$ is caused by viscous stresses at the interface between subchannels *i* and *j*.

### Lateral momentum conservation equation

!! Intentional comment to provide extra spacing

\begin{equation}
\label{lateral-momentum}
\frac{dw_{ij}}{dt} L_{ij} + \frac{L_{ij}}{\Delta Z} \Delta (w_{ij} \bar{U}) = - S_{ij}  \Delta P_{ij} + Friction_{ij}
\end{equation}

Here $g_{ij}$ is the gap between subchannels *i,j* and $\Delta Z$ the height of the control volume. Lateral pressure gradient ($\Delta P_{ij} / L_{ij}$) across the subchannels and/or forced mixing between subchannels owing to mixing vanes and spacer grids is the driving force behind diversion crossflow $w_{ij}$. $L_{ij}$ is the distance between the centers of subchannels *i,j*. $\bar{U_{ij}}$ is the average axial velocity of the two subchannels. The overall friction loss term $Friction_{ij}$ encompasses all the viscous effects and form losses associated with momentum exchange between the fluid and the wall due to the fluid motion through the gap.

### Enthalpy conservation equation

!! Intentional comment to provide extra spacing

\begin{equation}
\label{enthalpy-conservation}
\frac{d\left\langle \rho h\right\rangle_i }{dt}V_i + \Delta (\dot{m_i} h_i)  + \sum_{j} w_{ij} h^\star  + h'_{ij} = q'_i \Delta Z
\end{equation}

For a single-phase fluid, dissipation due to viscous stresses can be neglected and the total derivative of pressure (work of pressure) set to zero. Also there is no volumetric heat source due to moderation since heat is mainly transferred to the fluid through the fuel pins surface. $h'_{ij}$ is the turbulent enthalpy transfer between subchannels *i,j* and $q'_i$ is the average linear power $[\frac{kW}{m}]$ going into the control volumes of subchannel *i* from the fuel pins.

## Closure Models

!! Intentional comment to provide extra spacing

### Axial direction friction term

!! Intentional comment to provide extra spacing

\begin{equation}
Friction_i = -\frac{1}{2} K_i \frac{\dot{m_i} |\dot{m_i}|}{S_{i} \rho_i }
\end{equation}

where $K_{i} = [\frac{f_w}{Dhy_i} \Delta Z + k_i]$ is an overall axial loss coefficient encompassing local concentrated form losses $k_i$ due to the changing of the flow area or due to the narrowing of the surface area and frictional losses $\frac{f_w}{Dhy_i} \Delta Z$ due to fluid/pin interaction. $S_{i}$ is the axial flow area, $f_w = 4f$ is the Darcy friction factor and $Dhy_i = \frac{4 S_i}{P_w}$ is the hydraulic diameter.

### Lateral direction friction term

!! Intentional comment to provide extra spacing

\begin{equation}
Friction_{ij}  =  -\frac{1}{2} g_{ij} \Delta Z K_{ij} \rho_{} |u_{ij}| u_{ij} = - \frac{1}{2}K_{ij} \frac{w_{ij}|w_{ij}|}{S_{ij} \rho^\star}.
\end{equation}

where $K_{ij}$ is an overall loss coefficient encompassing lateral concentrated form and friction losses and $S_{ij}$ the lateral flow area between subchannel *i* and subchannel *j*: $S_{ij} = \Delta Z g_{ij}$, $\rho^*$ is the donor cell density.

### Friction factor

!! Intentional comment to provide extra spacing

The MATRA based friction factor for assemblies with bare pins in a quadrilateral lattice [!cite](KIT) is presented below.

\begin{equation}
f_w \rightarrow
\begin{cases}
64, & Re < 1\\
\frac{64}{Re}, &1 \leq Re<5000\\
0.316 Re^{-0.25}, &5000 \leq Re < 30000\\
0.184 Re^{-0.20}, &30000 \leq Re < 1e6
\end{cases}
\end{equation}

Additional friction factor models are implemented as follows:

- Quadrilateral assembly with bare pins: Chapter 9.6 Pressure drop in rod bundles [!cite](todreas2021nuclear1).
- Triangular assembly with bare pins: Chapter 9.6 Pressure drop in rod bundles [!cite](todreas2021nuclear1), The upgraded Cheng and Todreas correlation for pressure drop in hexagonal wire-wrapped rod bundles [!cite](chen2018upgraded).
- Triangular assembly with wire-wrapped pins: Chapter 9.6 Pressure drop in rod bundles [!cite](todreas2021nuclear1), The upgraded Cheng and Todreas correlation for pressure drop in hexagonal wire-wrapped rod bundles [!cite](chen2018upgraded).

### Turbulent momentum transfer

!! Intentional comment to provide extra spacing

The transfer of axial momentum due to turbulence is modelled as follows:

\begin{equation}
Drag_{ij} = -C_{T}\sum_{j} w_{ij}'\Delta U_{ij } = -C_{T}\sum_{j} w'_{ij}\bigg[ \frac{\dot{m_i}}{\rho_iS_i} - \frac{\dot{m_j}}{\rho_j S_j}\bigg].
\end{equation}

where $C_{T}$ is a turbulent modeling parameter.

### Turbulent enthalpy transfer

!! Intentional comment to provide extra spacing

The transfer of enthalpy due to turbulence is modelled as follows:

\begin{equation}
h_{ij}' = \sum_{j} w_{ij}'\Delta h_{ij} = \sum_{j} w'_{ij}\big[ h_i - h_j  \big].
\end{equation}

### Turbulent crossflow

!! Intentional comment to provide extra spacing

\begin{equation}
w_{ij}' = \beta S_{ij} \bar{G}, ~\frac{dw_{ij}'}{dz} = \frac{w_{ij}'}{\Delta Z}=\beta g_{ij} \bar{G}.
\end{equation}

where $\beta$ is the turbulent mixing parameter or thermal transfer coefficient and $\bar{G}$ is the average mass flux of the adjacent subchannels. The $\beta$ term is the tuning parameter for the mixing model. Physically, it is a non-dimensional coefficient that represents the ratio of the lateral mass flux due to mixing to the axial mass flux. It is used to model the effect of the unresolved scales of motion that are produced through the averaging process. In single-phase flow no net mass exchange occurs, both momentum and energy are exchanged between subchannels, and their rates of exchange are characterized in terms of hypothetical turbulent interchange flow rates ($w_{ij}^{'H},w_{ij}^{'M}$) [!cite](todreas2021nuclear2), for enthalpy and momentum respectively. The approximation that the rate of turbulent exchange for energy and momentum are related as follows is adopted: $w'_{ij} = w_{ij}^{'H} = w_{ij}^{'M} / C_T$.

 Additional turbulent mixing parameters are implemented as follows:

- Quadrilateral assembly with bare pins: A scale analysis of the turbulent mixing rate for various Prandtl number flow fields in rod bundles eq 25,Kim and Chung (2001) [!cite](kim2001scale), Modeling of flow blockage in a liquid metal-cooled reactor subassembly with a subchannel analysis code eq 19, Jeong et. al (2005)[!cite](jeong2005modeling).
- Triangular assembly with bare pins: A scale analysis of the turbulent mixing rate for various Prandtl number flow fields in rod bundles eq 25,Kim and Chung (2001) [!cite](kim2001scale).
- Triangular assembly with wire-wrapped pins: Hydrodynamic models and correlations for bare and wire-wrapped hexagonal rod bundles—bundle friction factors, subchannel friction factors and mixing parameters, Cheng and Todreas [!cite](cheng1986hydrodynamic).

### Calibrated parameter values

!! Intentional comment to provide extra spacing

$\beta$ has been calibrated for quadrilateral assemblies using data from the 2x3 air-water facility that was operated by Kumamoto university. The purpose of that facility was to quantify the effects of mixing and void drift [!cite](SADATOMI). In these experiments, the turbulent mixing rates and the fluctuations of static pressure difference between subchannels were measured. The author derived a way to use the die concentration measurements, in order to calculate  the turbulent mixing rates ($w_{ij}'$) between subchannels [!cite](SADATOMI2).

It is important to note that the mixing coefficient is simply a tuning parameter that will depend on the specific geometry of the facility being modeled. This facility is a square lattice, but the geometry is much larger than that of a typical PWR pin-lattice geometry. Nevertheless this study is useful for showing that the code is capable of predicting the correct mixing rate if it is calibrated correctly.

After calibrating the turbulent diffusion coefficient $\beta$ we turned our attention to the turbulent modeling parameter $C_{T}$. This is a tuning parameter that informs on how much momentum is transferred/diffused between subchannels, due to turbulence. The CNEN 4x4 test [!cite](Marinelli) performed at Studsvik laboratory for studying the flow mixing effect between adjacent subchannels was chosen to tune this parameter. This experiment consists in velocity and temperature measurements taken at the outlet of a 16-pin assembly test section. Analysis of the velocity distribution at the exit of the assembly can be used to calibrate the turbulent parameter $C_{T}$.

For quadrilateral assemblies: $C_{T} = 2.6$, $\beta = 0.006$ [!cite](kyriakopoulos2022development).

## Discretization

!! Intentional comment to provide extra spacing

The collocated discretization of the variables is presented in [fig:dis] . $i,j$ are the subchannel indexes. $ij$ is the name of the gap between subchannels $i,j$. $k$ is the index in the axial direction.

!media subchannel/getting_started/dis.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:dis
    caption=Subchannel collocated discretization.

The governing equations are discretized as follows:

- Conservation of mass:

\begin{equation}
\label{mass-dis}
\dot{m}_{i,k} - \dot{m}_{i,k-1} = - \sum_{j} w_{ij,k} - \frac{\rho_{i,k}^{n+1}V_{i,k} - \rho_{i,k}^n V_{i,k}}{\Delta t}
\end{equation}

    - The above equation can be written in matrix form as follows:

\begin{equation}
\begin{bmatrix}
1 & 0 & ... & 0\\
-1 & 1 & ... & 0 \\
: & : & ... & : \\
0 & ... &-1 & 1
\end{bmatrix}
\times
\begin{bmatrix}
\dot{m_{0,1}}  \\
\dot{m_{0,2}} \\
: \\
\dot{m_{i,k - 1}} \\
\dot{m}_{i,k}
\end{bmatrix} =
\begin{bmatrix}
\dot{m_{0,0}} - \sum_{j} w_{0j,1} - \frac{\rho_{0,1}^{n+1}V_{0,1} - \rho_{0,1}^n V_{0,1}}{\Delta t}\\
    - \sum_{j} w_{0j,2} - \frac{\rho_{0,2}^{n+1}V_{0,2} - \rho_{0,2}^n V_{0,2}}{\Delta t} \\
: \\
    - \sum_{j} w_{ij,k} - \frac{\rho_{i,k}^{n+1}V_{i,k} - \rho_{i,k}^n V_{i,k}}{\Delta t} \\
\end{bmatrix}
\end{equation}

which is equivalent to:

\begin{equation}
\label{mass-dis3}
\boldsymbol{M_{mm}} \vec{\dot{m}} = \vec{b_m} - \boldsymbol{M_{mw}}\vec{w}
\end{equation}

Similarly for the other equations,

- Conservation of linear momentum in the axial direction:

\begin{equation}
\label{axial-momentum-dis}
\Delta P_{i,k} = P_{i,k-1} - P_{i,k} = \frac{1}{S_{i,k-1}} \bigg[ \frac{\dot{m_{i,j}}^{n+1}  -  \dot{m}_{i,k}^{n}}{\Delta t} \Delta Z +
 \frac{\dot{m}_{i,k}^2}{S_{i,k} \rho_{i,k}} -  \frac{\dot{m}_{i,k-1}^2}{S_{i,k-1} \rho_{i,k-1}}
    + \sum_{j}w_{ij,k} U^\star  + C_{T}\sum_{j} w_{ij,k}' \big[ \frac{\dot{m}_{i,k}}{\rho_{i,k-1}S_{i,k}} - \frac{\dot{m_{j,k}}}{\rho_{jk-1} S_{j,k}}\big]
+\frac{1}{2} K_i \frac{\dot{m}_{i,k} |\dot{m}_{i,k}|}{S_{i,k} \rho_{i,k}} -g  \rho_{i,k} S_{i,k} \Delta Z \bigg]
\end{equation}
and in matrix form,
\begin{equation}
\boldsymbol{M_{pm}}(\vec{w}, \vec{\dot{m}})\vec{\dot{m}} =
\boldsymbol{S}\vec{\Delta P} + \vec{b_{P}} \\
\label{axial-momentum-dis3}
\boldsymbol{S}\vec{\Delta P} = -\boldsymbol{M_{pp}} \vec{P},
\end{equation}

where the matrix $\boldsymbol{M_{pm}}$ is calculated using the lagged values of the unknown variables $\vec{w}, \vec{\dot{m}}$.

- Conservation of linear momentum in the lateral direction:

\begin{equation}
\label{lateral-momentum-dis}
2S_{ij,k} L_{ij}\rho^*\frac{w_{ij,k}^{n+1} - w_{ij,k}^{n}}{\Delta t} + \frac{S_{ij,k} \rho^* L_{ij}}{\Delta Z} \bigg( \frac{\dot{m}_{i,k}}{S_{i,k-1} \rho_{i,k-1}} +  \frac{\dot{m_{j,k}}}{S_{j,k-1} \rho_{j,k-1}} \bigg) w_{ij,k}
    - \frac{S_{ij,k} \rho^*L_{ij}}{\Delta Z} \bigg( \frac{\dot{m}_{i,k-1}}{S_{i,k-1} \rho_{i,k-1}} +  \frac{\dot{m_{j,k-1}}}{S_{j,k-1} \rho_{j,k-1}} \bigg) w_{ij,k-1}  + K_{ij} w_{ij,k}|w_{ik,k}| - 2 S_{ij,k}^2 \rho^* \big[ P_{i,k-1} - P_{j,k-1}\big] = 0
\end{equation}

The above equation can be written in matrix form as follows:
\begin{equation}
\label{lateral-momentum-dis2}
\boldsymbol{M_{wp}}\vec{P} + \boldsymbol{M_{ww}}(\vec{\dot{m}}, \vec{w})\vec{w}= \vec{b_{w}}
\end{equation}
where the matrix $\boldsymbol{M_{ww}}$ is calculated using the lagged values of the unknown variables $\vec{w}, \vec{\dot{m}}$.

- Conservation of enthalpy:

\begin{equation}
\label{enthalpy-dis}
\frac{\rho_{i,k}^{n+1} h_{i,k}^{n+1} -  \rho_{i,k}^{n} h_{i,k}^{n}}{\Delta t}V_{i,k} + \dot{m}_{i,k}h_{i,k} - \dot{m}_{i,k-1}h_{i,k-1}
    + \sum_{j} w_{ij,k} h_k^\star
+\sum_{j} w_{ij,k}'\big[ h_{i,k-1} - h_{j,k-1}  \big] = \left\langle q' \right\rangle_{i,k} \Delta z_k -\sum_{j} Y_{ij,k} \frac{S_{ij,k} \eta_{ij,k}}{L_{ij,k}} (T_{i,k} - T_{j,k}) + \frac{Y_{i,k} S_{i,k} T_{i,k} - Y_{i,k-1} S_{i,k-1} T_{i,k-1}}{\Delta z_k}
\end{equation}

The above equation can be written in matrix form as follows:

\begin{equation}
\label{enthalpy-dis2}
 \boldsymbol{M_{hh}}(\vec{\dot{m}}, \vec{w}) \vec{h} = \vec{b_h}
\end{equation}
where the matrix $\boldsymbol{M_{hh}}$ is calculated using the lagged values of the unknown variables $\vec{w}, \vec{\dot{m}}$.

## Algorithm

!! Intentional comment to provide extra spacing

A hybrid numerical method of solving the subchannel equations was developed. Hybrid in this context means that the user has the option of solving each portion of the problem at a time, by dividing the domain into blocks. Each block is solved sequentially from inlet to outlet. The mass flow at the outlet of the previous block and the pressure at the inlet of the next block provide the needed boundary conditions. The essence of the algorithm hinges on the construction of a combined residual function based on the lateral momentum equation. To solve this equation a Jacobian Free Newton-Krylov type Method (JFNKM) was used. The workhorse of the code is the non linear equation solvers (SNES) found in the Portable, Extensible Toolkit for Scientific Computation [PETSc](https://petsc.org/release/).

\begin{equation}
\label{lateral1}
f(w_{ij}) = \frac{dw_{ij}}{dt} L_{ij} + \frac{L_{ij} }{\Delta z} \Delta (w_{ij} \bar{U })  - S_{ij} \Delta P_{ij} + \frac{1}{2} K_{ij}
\frac{w_{ij}|w_{ij}|}{\rho^*}= 0.
\end{equation}

The main unknown variable in this non linear residual is the crossflow $w_{ij}$. The combined residual function calculates the non linear residual $f(w_{ij})$ after it updates the other main flow variables, such as mass flow $\dot{m}_i$,  turbulent crossflow $w'_{ij}$, pressure drop  $\Delta P_i$ and pressure $P_i$, using the current $w_{ij}$ as needed. So every time this function is called by the Newton solver the flow variables get updated. This affords the solution of all flow variables at the same time. $P_i$ is the local pressure minus the exit pressure, $P_i (z) - P_{exit}$, so at the exit $P_{i}$ is zero. The hybrid algorithm is presented in [stencil].

!media subchannel/getting_started/stencil.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=stencil
    caption=SCM hybrid numerical scheme

Once the main flow variables converge in a block, the enthalpy conservation equation is solved and enthalpy $(h)$ is retrieved in all the nodes of the block. In the special case where no heat is added to the fluid in the block, the enthalpy does not need to be calculated in that block (unless there is a non-uniform enthalpy inlet distribution). Using enthalpy, pressure and the equations of state, temperature $T_i$ and the fluid properties such as density $\rho_i$ and viscosity $\mu_i$ are calculated. After the fluid properties are updated, the solve is repeated until the temperature field converges. Once the temperature solution converges the procedure is repeated for the next block downstream. Once the temperature solution converges in all blocks we check to see if pressure has converged in all blocks. If not, we repeat the procedure starting again from the first block, until pressure has converged. Note that in order for the pressure information from the outlet to reach the inlet, it will require a number of the pressure loop iterations equal to the number of blocks. Last, the calculation of the flow variables and of the residual is done in an explicit manner.

### Algorithm variations

!! Intentional comment to provide extra spacing

There are three variations [!cite](kyriakopoulos2022development), [!cite](kyriakopoulos2023demonstration) of the algorithm presented above. There should be no appreciable differences between the results of the algorithms when the time/spacial discretization scheme is converged.

#### Explicit

!! Intentional comment to provide extra spacing

This is the default algorithm, where the unknown flow variables are calculated in an explicit manner through their governing equations. The variables are updated sequantially from block inlet to block outlet except for pressure which is updated from block outlet to block inlet. Blocks are solved sequentially from assembly inlet to assembly outlet.

#### Implicit segregated

!! Intentional comment to provide extra spacing

In this case, the governing mass, axial momentum and crossflow momentum, equations are recast in matrix form and the flow variables are calculated by solving the corresponding system. This means that variables are retrieved concurrently for the whole block. Otherwise, the solution algorithm is the same as in the default explicit method.

#### Implicit

!! Intentional comment to provide extra spacing

In this case, the governing mass, axial momentum and crossflow momentum  conservation equations are recast in matrix form and combined into a single system. The system of all the subchannel equations looks like this:

\begin{equation}
\begin{bmatrix}
\boldsymbol{M_{mm}} & 0 & \boldsymbol{M_{mw}} & 0\\
\boldsymbol{M_{pm}} & \boldsymbol{M_{pp}} & 0 & 0 \\
0 & \boldsymbol{M_{wp}} & \boldsymbol{M_{ww}} & 0 \\
0 & 0 & 0 & \boldsymbol{M_{hh}}
\end{bmatrix}
\times
\begin{bmatrix}
\vec{\dot{m}} \\
\vec{P} \\
\vec{w}\\
\vec{h}
\end{bmatrix} =
\begin{bmatrix}
\vec{b_m}\\
\vec{b_p} \\
\vec{b_w} \\
\vec{b_h}
\end{bmatrix}
\end{equation}

Since the enthalpy governing equations are uncoupled from the other equations in this otherwise monolithic system (enthalpy is coupled to the flow equations via the fluid properties update), it makes sense to lag the enthalpy solution and solve for it separately. The flow variables are calculated by solving that big system (without the enthalpy) to retrieve all the unknowns at the same time instead of one by one, and on all the nodes of the block: $\vec{\dot{m}}, \vec{P}, \vec{w_{ij}}$. The solution algorithm is the same as in the default method and the solver used is PETSc KSPSolve.

As soon as the big matrix is constructed, the solver will calculate cross-flow resistances to maintain realizability. A distinctive feature of this method is the introduction of a *weak relaxation* logic that stabilizes and accelerates convergence of the coupled $mass flow: (\dot{\mathbf{m}})$, $pressure: (\mathbf{P})$, and $crossflow:(\mathbf{w}_{ij})$ fields in a $Q{=}3$ block-nested linear system with matrix blocks $M_{ij}$ and right-hand-side blocks $\mathbf{b}_i$ that represent the individual governing equations. Note that the solution is influenced by the stabilization method and its coefficients.

#### 1. Fast scale estimates

!! Intentional comment to provide extra spacing

From the axial- and cross-momentum rows, the code forms quick, diagonally preconditioned estimates:
\begin{equation}
\begin{aligned}
\hat{\mathbf m} &= M_{pm}\,\mathbf m, \\
\hat{\mathbf p} &= \frac{\hat{\mathbf m}}{\operatorname{diag}(M_{pp}) + \varepsilon_p\mathbf 1},\\
\hat{\mathbf W} &= \frac{M_{wp}\,\hat{\mathbf p} - \mathbf b_w}{\operatorname{diag}(M_{ww}) + \varepsilon_W\mathbf 1},
\end{aligned}
\end{equation}
with small safeguards $\varepsilon_p,\varepsilon_W\sim 10^{-10}$ to avoid division by zero. Using $\hat{\mathbf W}$, the per-channel crossflow sum $\sum_{j} w_{ij}$ is assembled into a vector $\mathrm{sumw_{ij}}_{\mathrm{loc}}$.

#### 2. Crossflow relaxation parameter

!! Intentional comment to provide extra spacing

Two guarded scalars are computed:

\begin{equation}
\begin{aligned}
m_{\min} &= \max\big(\min |\mathbf m|,\; 10^{-10}\big),\\
S_{\max} &= \max\Big(\max |\mathrm{sumw_{ijloc}}|,\; 10^{-10}\Big)
\end{aligned}
\end{equation}

Additionally, a mean inter-iteration change for crossflow is formed
\begin{equation}
r_{\mathrm{base}} = \operatorname{mean}\big(\big|\mathbf W^{(k)}| - |\mathbf W^{(k-1)}\big|\big),
\end{equation}
leading to a relaxation factor
\begin{equation}
r = \frac{r_{\mathrm{base}}}{\max(S_{\max}, \varepsilon)} + 0.5,\qquad \varepsilon\sim10^{-10}.
\end{equation}
The +0.5 offset biases toward mild under-relaxation.

#### 3. Crossflow resistance inflation

!! Intentional comment to provide extra spacing

A cross-coupling resistance is estimated and smoothed:
\begin{equation}
\begin{aligned}
\tilde K   &= \frac{S_{\max}}{m_{\min}}, &
K^\star &= 0.9\,\tilde K + 0.1\,K_{\text{old}}, &
K       &= r\,K^\star.
\end{aligned}
\end{equation}
After smoothing, the provisional crossflow resistance $K$ is mapped through a piecewise lower-bound function that enforces minimum safe damping levels in specific ranges.

\begin{equation}
K \rightarrow
\begin{cases}
K , & K >= 10, \\
1.0, & 1 \leq K < 10, \\
0.5, & 0.1 \leq K < 1, \\
\frac{1}{3}, & 0.01 \leq K < 0.1, \\
0.1, & 0.001 \leq K < 0.01, \\
K, & K < 10^{-3}.
\end{cases}
\end{equation}

This mapping acts as a {snap-up} rule for the crossflow resistance $K$ over the range $[10^{-3}, 10]$:
it raises $K$ out of weak-damping intervals but leaves very small and very large
values unchanged. The purpose is to maintain numerical stability and adequate
diagonal dominance in the cross-momentum equations without introducing full quantization or "bucketing".

Finally, $K$ is added to the diagonal of the cross-momentum block,
\begin{equation}
M_{ww} \;\leftarrow\; M_{ww} + K\,I,
\end{equation}
thereby increasing diagonal dominance and improving conditioning for the crossflow equations. Note that this treatment does influence the cross-flow distribution solution.

#### 4. Per-equation under-relaxation

!! Intentional comment to provide extra spacing

Classical linear under-relaxation is applied automatically and separately to each equation $f\in\{\mathbf m,\mathbf p,\mathbf W\}$ using factors
\begin{equation}
\alpha_m=1.0,\qquad \alpha_p=1.0,\qquad \alpha_W=0.1.
\end{equation}
For each equation, with the corresponding diagonal $D_f=\operatorname{diag}(M_{ff})$, the system is modified as
\begin{equation}
\begin{aligned}
M_{ff} &\leftarrow \frac{1}{\alpha_f} M_{ff}, \\
\mathbf b_f &\leftarrow \mathbf b_f + (1-\alpha_f)\,D_f\,\mathbf x^{\text{old}}_f.
\end{aligned}
\end{equation}
This standard construction ensures that solving the modified linear system yields the *under-relaxed* update for equation $f$. In practice, only $\mathbf W$ is strongly damped, while $\mathbf m$ and $\mathbf p$ can be solved without additional damping. This relaxation happens inside the temperature loop.

#### 5. Net effect

!! Intentional comment to provide extra spacing

The combination of (i) safeguarded scale estimation, (ii) adaptive, time smoothed, and piecewise snapped added crossflow resistance, and (iii) selective under-relaxation produces a more diagonally dominant and robust nested solve that tolerates rapid changes in crossflow while preserving good convergence properties for mass flow and pressure.
