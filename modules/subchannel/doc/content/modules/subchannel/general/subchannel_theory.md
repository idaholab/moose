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

The MATRA based friction factor for assemblies with bare pins in a quadrilateral lattice [!cite](KIT) is presented below. For Reynolds number ranges below $Re = 5000$, where the MATRA correlation is not applicable, SCM applies a custom extension that keeps the friction factor continuous at the transition to the MATRA correlation:

\begin{equation}
Re_c = \left(\frac{64}{0.316}\right)^{4/3},
\quad
\eta = \frac{Re - Re_c}{5000 - Re_c},
\quad
w = 3 \eta^2 - 2 \eta^3
\end{equation}

\begin{equation}
f_w \rightarrow
\begin{cases}
64, & Re < 1\\
\frac{64}{Re}, &1 \leq Re < Re_c\\
(1 - w)\frac{64}{Re} + w 0.316 Re^{-0.25}, &Re_c \leq Re < 5000\\
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

where $\beta$ is the turbulent mixing parameter or thermal transfer coefficient and $\bar{G}$ is the average mass flux of the adjacent subchannels. The $\beta$ term is the tuning parameter for the mixing model. Physically, it is a non-dimensional coefficient that represents the ratio of the lateral mass flux due to mixing to the axial mass flux. It is used to model the effect of the unresolved scales of motion that are produced through the averaging process. In single-phase flow no net mass exchange occurs, both momentum and energy are exchanged between subchannels, and their rates of exchange are characterized in terms of hypothetical turbulent interchange flow rates ($w_{ij}^{'H},w_{ij}^{'M}$) [!cite](todreas2021nuclear2), for enthalpy and momentum respectively. For this unresolved turbulent interchange model, the approximation that the rate of turbulent exchange for energy and momentum are related as follows is adopted: $w'_{ij} = w_{ij}^{'H} = w_{ij}^{'M} / C_T$.

The mixing closure provides only this unresolved turbulent interchange coefficient. Friction closures enter the axial and lateral momentum equations through the pressure-drop terms, and heat-transfer closures enter the energy equation through pin/duct heat addition. When a mixing closure requires local flow information such as a friction factor, the selected friction closure is used internally by that empirical mixing correlation; otherwise $\beta$ is coupled to the governing equations only through $w'_{ij}$ and the optional $C_T$ momentum scaling above.

### Sweep flow

!! Intentional comment to provide extra spacing

The turbulent interchange relation above is not applicable to wire-wrap sweep flow. In wire-wrapped triangular assemblies, sweep flow represents a directed peripheral enthalpy transport induced by the wire wrap. SCM applies the sweep-flow coefficient only in the triangular-assembly energy equation, where the term transports enthalpy between edge and corner subchannels. It is not included in the momentum exchange term and is not scaled by $C_T$; equivalently, SCM does not currently model a corresponding momentum sweep-flow closure.

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

### Time grid discretization

SCM uses a first-order backward Euler discretization for all temporal storage terms. Storage is evaluated at
the downstream node of each axial control volume. Because this time discretization is implemented
directly by SCM, selecting a MOOSE time integrator other than `ImplicitEuler` does not change it;
SCM issues a warning when another time integrator is requested.

### Spatial discretization

The collocated discretization of the variables is presented in [fig:dis] . $i,j$ are the subchannel indexes. $ij$ is the name of the gap between subchannels $i,j$. $k$ is the index in the axial direction.

!media subchannel/getting_started/dis.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:dis
    caption=Subchannel collocated discretization.

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

For a segregated solve, this is equivalent to:

\begin{equation}
\label{mass-dis3}
\boldsymbol{M_{mm}} \vec{\dot{m}} = \vec{b_m} - \boldsymbol{M_{mw}}\vec{w}
\end{equation}

In the transient monolithic solve, the new-time density in the storage term is also linearized with
respect to pressure at constant enthalpy:

\begin{equation}
\left(\frac{\partial \rho}{\partial p}\right)_h =
\left(\frac{\partial \rho}{\partial p}\right)_T -
\left(\frac{\partial \rho}{\partial T}\right)_p\,
\frac{\left(\frac{\partial h}{\partial p}\right)_T}
{\left(\frac{\partial h}{\partial T}\right)_p}.
\label{density-pressure-derivative}
\end{equation}

The corresponding pressure block gives the monolithic mass equation

\begin{equation}
\label{mass-dis-monolithic}
\boldsymbol{M_{mm}}\vec{\dot{m}} +
\boldsymbol{M_{mp}}\vec{P} +
\boldsymbol{M_{mw}}\vec{w} = \vec{b_m}.
\end{equation}

Here $\boldsymbol{M_{mp}}$ is assembled when density and energy are computed in a transient
monolithic solve and is zero for a steady solve. Its entries may also vanish for fluid models
without pressure-dependent density or enthalpy. For fluids with a nonzero constant-enthalpy
pressure derivative, the block captures an $\mathcal{O}(1/\Delta t)$ pressure-density coupling that
would otherwise remain in the outer fixed-point iteration. In the current
[PBSodiumFluidProperties.md] model, both $(\partial \rho / \partial p)_T$ and
$(\partial h / \partial p)_T$ are zero, so $\boldsymbol{M_{mp}}=0$.

Similarly for the other equations,

- Conservation of linear momentum in the axial direction:

\begin{equation}
\label{axial-momentum-dis}
\Delta P_{i,k} = P_{i,k-1} - P_{i,k} = \frac{1}{S_{i,k-1}} \bigg[ \frac{\dot{m}_{i,k}^{n+1}  -  \dot{m}_{i,k}^{n}}{\Delta t} \Delta Z +
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

where the matrix $\boldsymbol{M_{pm}}$ is calculated using the lagged values of the unknown variables
$\vec{w}, \vec{\dot{m}}$. The axial-momentum storage term uses $\dot{m}_{i,k}$ at the downstream
node for both the new and old time levels.

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

The main unknown variable in this non linear residual is the crossflow $w_{ij}$. The combined residual function calculates the non linear residual $f(w_{ij})$ after it updates the other main flow variables, such as mass flow $\dot{m}_i$,  turbulent crossflow $w'_{ij}$, pressure drop  $\Delta P_i$ and pressure $P_i$, using the current $w_{ij}$ as needed. So every time this function is called by the Newton solver the flow variables get updated. This affords the solution of all flow variables at the same time. $P_i$ is the local pressure minus the exit pressure, $P_i (z) - P_{exit}$, so at the exit $P_{i}$ is zero. The hybrid algorithm is presented in [scm-solver-flowchart].

!media media_scripts/scm_solver_flowchart.py
    style=width:60%;margin-bottom:2%;margin:auto;
    id=scm-solver-flowchart
    caption=SCM solver iteration scheme

For each outer pressure iteration, blocks are visited sequentially from the assembly inlet to the
outlet. Within a block, SCM first refreshes the flow solution and then solves the enthalpy equation,
recovers temperature from pressure and enthalpy, and updates density and viscosity. The
`enthalpy_subcycles` parameter controls how many enthalpy, temperature, and property updates are
performed before the next flow solve. Its default value of one preserves the original
flow-then-enthalpy ordering. Values greater than one opt into thermal subcycling with a lagged flow
field.

The temperature recovered from the equation of state can be relaxed independently:

\begin{equation}
\vec{T}^{\,\ell+1} =
\vec{T}^{\,\ell} +
\alpha_T\left[\vec{T}(\vec{p},\vec{h})-\vec{T}^{\,\ell}\right],
\end{equation}

where $\alpha_T$ is set by `T_relaxation` and defaults to one. The temperature convergence measure
uses the unrelaxed equation-of-state update,

\begin{equation}
\epsilon_T =
\frac{\left\|\vec{T}(\vec{p},\vec{h})-\vec{T}^{\,\ell}\right\|_2}
{\left\|\vec{T}^{\,\ell}\right\|_2 + 10^{-14}},
\end{equation}

so changing `T_relaxation` does not redefine `T_tol`.

After all blocks have been processed, SCM checks pressure convergence. The segregated algorithms
use the relative field change

\begin{equation}
\epsilon_P =
\frac{\left\|\vec{P}^{\,\ell+1}-\vec{P}^{\,\ell}\right\|_2}
{\left\|\vec{P}^{\,\ell}+P_{\mathrm{out}}\mathbf{1}\right\|_2+10^{-14}},
\end{equation}

whereas the monolithic algorithm uses the largest unrelaxed pressure fixed-point update among the
blocks. Measuring the monolithic update before post-solve relaxation keeps the meaning of `P_tol`
independent of `pressure_relaxation`. The maximum errors are synchronized across processes. If the
pressure field has not converged, the block sweep starts again at the inlet; pressure information
therefore requires multiple outer iterations to propagate upstream when several blocks are used.
`P_maxit` and `T_maxit` limit the outer and thermal iterations, respectively; `P_maxit = 0` selects
the solver's automatic outer-iteration limit.

### Algorithm variations

There are three variations [!cite](kyriakopoulos2026numerical) of the algorithm presented above in SCM: The Explicit-Segregated, Implicit-Segregated and Monolithic. There are also two subchannel geometries that SCM can solve, one with fuel pins in a quadrilateral lattice (Quadsolver) and one with fuel pins in a triangular lattice (Trisolver). There should be no appreciable differences between the results of the algorithms when the time/spacial discretization scheme is converged.

#### Explicit-Segregated

!! Intentional comment to provide extra spacing

This is the default algorithm, where the unknown flow variables are calculated in an explicit manner through their governing equations. The variables are updated sequantially from block inlet to block outlet except for pressure which is updated from block outlet to block inlet. Blocks are solved sequentially from assembly inlet to assembly outlet.

#### Implicit-Segregated

!! Intentional comment to provide extra spacing

In this case, the governing mass, axial momentum and crossflow momentum, equations are recast in matrix form and the flow variables are calculated by solving the corresponding system. This means that variables are retrieved concurrently for the whole block. Otherwise, the solution algorithm is the same as in the default explicit method.

#### Monolithic

!! Intentional comment to provide extra spacing

In this case, the governing mass, axial momentum and crossflow momentum  conservation equations are recast in matrix form and combined into a single system. The system of all the subchannel equations looks like this:

\begin{equation}
\begin{bmatrix}
\boldsymbol{M_{mm}} & \boldsymbol{M_{mp}} & \boldsymbol{M_{mw}} & 0\\
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

The pressure coupling $\boldsymbol{M_{mp}}$ is the transient density linearization described in
[density-pressure-derivative]. Since the enthalpy governing equations are uncoupled from the other
equations in this otherwise monolithic system (enthalpy is coupled to the flow equations through
the fluid-property update), enthalpy is lagged and solved separately. The flow system retrieves
$\vec{\dot{m}}$, $\vec{P}$, and $\vec{w}$ concurrently at every node in a block; $\vec{\Delta P}$
is not explicitly calculated. The coupled flow system is solved with PETSc FGMRES and a field-split
preconditioner. SCM checks the PETSc convergence reason for both the coupled flow and enthalpy
linear solves and reports the reason, iteration count, and residual norm instead of accepting a
diverged solution.

As soon as the big matrix is constructed, the solver will calculate cross-flow resistances to maintain realizability. A distinctive feature of this method is the introduction of a *weak relaxation* logic that stabilizes and accelerates convergence of the coupled $mass flow: (\dot{\mathbf{m}})$, $pressure: (\mathbf{P})$, and $crossflow:(\mathbf{w}_{ij})$ fields in a $Q{=}3$ block-nested linear system with matrix blocks $M_{ij}$ and right-hand-side blocks $\mathbf{b}_i$ that represent the individual governing equations. Note that the solution is influenced by the stabilization method and its coefficients.

#### 1. Fast scale estimates

!! Intentional comment to provide extra spacing

From the axial- and cross-momentum rows, the code forms a quick pressure estimate and a provisional
cross-momentum imbalance:
\begin{equation}
\begin{aligned}
\hat{\mathbf m} &= M_{pm}\,\mathbf m, \\
\hat{\mathbf p} &= \frac{\hat{\mathbf m}}{\operatorname{diag}(M_{pp}) + \varepsilon_p\mathbf 1},\\
\hat{\mathbf r}_w &= M_{wp}\,\hat{\mathbf p} - \mathbf b_{w,p},
\end{aligned}
\end{equation}
where $\mathbf b_{w,p}$ is the pressure-force right-hand side and
$\varepsilon_p=10^{-10}$ avoids division by zero. The signed gap contributions in
$\hat{\mathbf r}_w$ are accumulated per channel into
$\mathrm{sumw_{ij}}_{\mathrm{loc}}$.

#### 2. Adaptive resistance multiplier

!! Intentional comment to provide extra spacing

Two scales are computed:

\begin{equation}
\begin{aligned}
m_{\min} &= \min |\mathbf m|,\\
S_{\max} &= \max\Big(\max |\mathrm{sumw_{ijloc}}|,\; 10^{-10}\Big)
\end{aligned}
\end{equation}

Additionally, a mean inter-iteration change for crossflow is formed
\begin{equation}
r_{\mathrm{base}} = \operatorname{mean}\big(\big|\mathbf W^{(k)}| - |\mathbf W^{(k-1)}\big|\big),
\end{equation}
leading to an adaptive resistance multiplier
\begin{equation}
r = \frac{r_{\mathrm{base}}}{\max(S_{\max}, \varepsilon)} + 0.5,\qquad \varepsilon\sim10^{-10}.
\end{equation}
The +0.5 offset supplies a baseline contribution to the added resistance.

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
thereby increasing diagonal dominance and improving conditioning for the crossflow equations. Note
that this treatment does influence the crossflow distribution solution.

#### 4. Equation under-relaxation

!! Intentional comment to provide extra spacing

Classical linear under-relaxation is applied separately to each equation
$f\in\{\mathbf m,\mathbf p,\mathbf W\}$. The factors are user-selectable through
`mass_flow_equation_relaxation`, `pressure_equation_relaxation`, and
`crossflow_equation_relaxation`; their defaults are
\begin{equation}
\alpha_m=1.0,\qquad \alpha_p=1.0,\qquad \alpha_W=0.1.
\end{equation}
For each equation, with $D_f=\operatorname{diag}(M_{ff})$, only the diagonal and right-hand side
are modified:
\begin{equation}
\begin{aligned}
M_{ff} &\leftarrow M_{ff} +
\left(\frac{1}{\alpha_f}-1\right)D_f, \\
\mathbf b_f &\leftarrow \mathbf b_f +
\left(\frac{1}{\alpha_f}-1\right)D_f\,\mathbf x^{\text{old}}_f.
\end{aligned}
\end{equation}
The off-diagonal entries are unchanged. A factor of one bypasses relaxation, while a factor below
one increases the diagonal magnitude and damps the update toward the previous iterate without
changing the fixed point. With the defaults, only the crossflow equation is under-relaxed.

#### 5. Post-solve solution relaxation

!! Intentional comment to provide extra spacing

After the coupled system is solved, each solution field can be relaxed independently:

\begin{equation}
\mathbf x_f^{\,\ell+1} =
\beta_f\mathbf x_f^\star + (1-\beta_f)\mathbf x_f^{\,\ell},
\end{equation}

where $\mathbf x_f^\star$ is the raw linear solution. The factors are set with
`mass_flow_relaxation`, `pressure_relaxation`, and `crossflow_relaxation`; all three default to
one. Equation relaxation and post-solve relaxation are independent and may be used together. Both
preserve the fixed point: equation relaxation modifies the matrix and right-hand side before the
linear solve, while post-solve relaxation damps the fixed-point update after that solve.

#### 6. Net effect

!! Intentional comment to provide extra spacing

The combination of (i) scale estimation, (ii) adaptive, iteration-smoothed, and piecewise snapped
added crossflow resistance, and (iii) independently configurable equation and solution relaxation
improves robustness of the nested solve during rapid crossflow changes. Added resistance and
equation relaxation both increase entries on the crossflow diagonal, but neither guarantees strict
diagonal dominance for every geometry and flow state. Post-solve relaxation does not alter matrix
conditioning. Users can retain the default behavior or tune the two relaxation layers separately.
