# Cold CO$_{2}$ injection into an elastic reservoir - a multi-phase THM problem

Andy Wilkins, Chris Green, Jonathan Ennis-King

Two papers of LaForce et al. [!citep](laforce2014a,laforce2014b) derive semi-analytical solutions of a 2-phase THM problem involving injection of a cold fluid into a warmer, elastic reservoir depicted in [tara_setup_fig].
In this page, their solution will be replicated in MOOSE.  The nomenclature used here is described in a [separate page](nomenclature.md).

!media media/porous_flow/tara_setup.png style=width:30% caption=The geometrical setup used in the papers of LaForce et al.  id=tara_setup_fig

LaForce et al. consider a cylindrically-symmetric model, as depicted in [tara_setup_fig], with radial coordinate labelled by $r$ and axial coordinate $z$.  A permeable reservoir is sandwiched between two impermeable seals.  The reservoir and seals are oriented horizontally (normal to $z$).  The reservoir is initially fully water saturated.

A vertical wellbore intersects the reservoir and injects CO$_{2}$ into it at a constant rate.  The CO$_{2}$ is colder than the initial reservoir temperature.  The injected fluid advects through the reservoir (but not through the impermeable seals).  Heat (or cold) advects with the fluid, slowly cooling the reservoir, and conducting into the surrounding seals.

The PorousFlow input file and associated scripts that generate the analytic solutions are [here](https://github.com/idaholab/moose/blob/master/modules/porous_flow/examples/thm_example).  The input file describes an axially symmetric situation:

!listing modules/porous_flow/examples/thm_example/2D.i block=Mesh

!listing modules/porous_flow/examples/thm_example/2D.i block=Problem



## Equations and assumptions

To derive the semi-analytical solutions, LaForce et al. employ the assumptions stated below.

### Fluid flow: mass

In PorousFlow, the mass of species $\kappa$ per volume of rock is written as a sum
over all phases present in the system:
\begin{equation}
\label{eqn.msph}
M^{\kappa} =
\phi\sum_{\beta}S_{\beta}\rho_{\beta}\chi_{\beta}^{\kappa}
+ (1 - \phi)C^{\kappa}
\end{equation}
LaForce et al. make the following assumptions

- The porosity, $\phi$, is constant.  It is independent of time, fluid pressure, temperature and volumetric expansion.  However, it is dependent on rock type.

!listing modules/porous_flow/examples/thm_example/2D.i start=[porosity_reservoir] end=[permeability_reservoir]

- There are two phases, liquid and gas.  Hence $\beta \in \{\mathrm{liquid, gas}\}$.

- The liquid is water and the gas is CO$_{2}$.  Hence $\kappa \in \{ \mathrm{water, co2}\}$.  The water component exists only in the liquid phase and the CO$_{2}$ component exists only in the gas phase.  Hence $\chi_{\mathrm{liquid}}^{\mathrm{water}}=1$, $\chi_{\mathrm{liquid}}^{\mathrm{co2}}=0$, $\chi_{\mathrm{gas}}^{\mathrm{water}}=0$ and $\chi_{\mathrm{gas}}^{\mathrm{co2}}=1$.

!listing modules/porous_flow/examples/thm_example/2D.i start=[massfrac_ph0_sp0] end=[pgas]

- The density of each phase is constant.   This is in order to simplify the fluid equations, which eventually reduce to a well-studied form that models advection of a saturation shock front.  However, in order to derive solutions for the pressure distribution, LaForce et al. assume a small fluid compressibility, which is the same for each phase.  To satisfy both the incompressibility and small compressibility requirements, the MOOSE model uses a fluid bulk modulus that is $\sim 10^{7}$ larger than any porepressures.

- There is no desorbed fluid.

With these assumptions, the mass formula becomes
\begin{equation}
\begin{aligned}
  M^{\mathrm{water}} & = \phi
  S_{\mathrm{water}}\rho_{\mathrm{water}} \ , \\
  M^{\mathrm{co2}} & =  \phi
  (1 - S_{\mathrm{water}})\rho_{\mathrm{co2}} \ .
\end{aligned}
\end{equation}

The PorousFlow input file uses the porepressure of water and the gas saturation as its primary nonlinear variables describing fluid flow:

!listing modules/porous_flow/examples/thm_example/2D.i start=[pwater] end=[temp]


### Fluid flow: flux

In PorousFlow, the flux is a sum of advective flux and diffusive-and-dispersive flux:
\begin{equation}
\label{adv.diff.disp.eqn}
\mathbf{F}^{\kappa} =
\sum_{\beta}\chi_{\beta}^{\kappa}\mathbf{F}_{\beta}^{\mathrm{advective}}
+ \mathbf{F}^{\kappa}_{\mathrm{diffusion+dispersion}} \ .
\end{equation}
Advective flux is governed by Darcy's law:
\begin{equation}
\mathbf{F}_{\beta}^{\mathrm{advective}} = \rho_{\beta}\mathbf{v}_{\beta} =
-\rho_{\beta}\frac{k\,k_{\mathrm{r,}\beta}}{\mu_{\beta}}(\nabla
P_{\beta} - \rho_{\beta} \mathbf{g})
\label{darc.eqn}
\end{equation}
Diffusion and dispersion are proportional to the gradient of
$\chi_{\beta}^{\kappa}$:
\begin{equation}
\mathbf{F}^{\kappa}_{\mathrm{diffusion+dispersion}} =
-\sum_{\beta}\rho_{\beta}{\mathcal{D}}_{\beta}^{\kappa}\nabla
\chi_{\beta}^{\kappa}
\label{diff.disp.eqn}
\end{equation}

LaForce et al. make the following assumptions

- The permeability tensor is diagonal and constant (independent of fluid pressure, temperature and rock stress and strain).  Its $xx$ and $yy$ components are equal, and its $zz$ component is zero.  It is dependent on rock type.  Since the PorousFlow input file is setup in 2D RZ coordinates, the radial permeability enters the first "slot", the $zz$ component appears in the central slot, and all other components are zero in this situation

!listing modules/porous_flow/examples/thm_example/2D.i start=[permeability_reservoir] end=[relperm_liquid]

- There is a constant and nonzero residual water saturation $S_{\mathrm{water}}^{\mathrm{res}}$ and constant and nonzero residual CO$_{2}$ saturation $S_{\mathrm{co2}}^{\mathrm{res}}$.

- The relative permeability functions are functions of the effective saturation
  \begin{equation}
    \hat{S} = \frac{S_{\mathrm{water}} -
      S_{\mathrm{water}}^{\mathrm{res}}}{1 -
      S_{\mathrm{water}}^{\mathrm{res}} -
      S_{\mathrm{co2}}^{\mathrm{res}}}
  \end{equation}
  and the functions are
  \begin{equation}
    \label{relperms.eqns}
  \begin{aligned}
    k_{\mathrm{r,\,water}} & = \hat{S}^{4} \ , \\
    k_{\mathrm{r,\,co2}} & = (1 - \hat{S})^{2}(1 - \hat{S}^{2}) \ .
  \end{aligned}
  \end{equation}

!listing modules/porous_flow/examples/thm_example/2D.i start=[relperm_liquid] end=[thermal_conductivity_reservoir]

- The viscosity is a function of temperature only.  LaForce et al. use a particular function of temperature that is not included in PorousFlow.  Therefore, here we shall use constant viscosity for each phase that is equal to the average viscosity quoted in LaForce et al., and use LaForce et al.'s solution based on that constant viscosity.  This results in only very tiny modifications to LaForce's solutions which would be difficult to see in the finite-resolution of the MOOSE model anyway.

- There is no capillarity.  Therefore $P_{\mathrm{water}}=P_{\mathrm{co2}}$.  Define $P=P_{\mathrm{water}}$.

!listing modules/porous_flow/examples/thm_example/2D.i start=[pc] end=[]

- There is no gravity.

- There is no diffusion and dispersion (which is implicitly implied by the assumptions on $\chi$).

With these assumptions, the fluid fluxes are
\begin{equation}
\begin{aligned}
  \mathbf{F}^{\mathrm{water}} & = -\rho_{\mathrm{water}} \frac{k
    k_{\mathrm{r\,water}}}{\mu_{\mathrm{water}}}\nabla P \ , \\
  \mathbf{F}^{\mathrm{co2}} & = -\rho_{\mathrm{co2}} \frac{k
    k_{\mathrm{r\,co2}}}{\mu_{\mathrm{co2}}}\nabla P \ .
\end{aligned}
\end{equation}

### Fluid flow: mass conservation

In PorousFlow, mass conservation for fluid species $\kappa$ is described by the continuity
equation
\begin{equation}
\label{mass.cons.sp.eqn}
0 = \frac{\partial M^{\kappa}}{\partial t} + M^{\kappa}\nabla\cdot{\mathbf
  v}_{s} + \nabla\cdot \mathbf{F}^{\kappa} + \Lambda M^{\kappa} - q^{\kappa} \ .
\end{equation}

LaForce et al. make the following assumptions:

- The term $\nabla\cdot{\mathbf{v}}_{s}$ is ignored.

- There is no radioactive decay, $\Lambda = 0$.

- Sources and sinks only occur on the boundary, so $q = 0$ except for on the boundary.

Therefore, the fluid-flow `Kernels` are

!listing modules/porous_flow/examples/thm_example/2D.i start=[mass_water_dot] end=[energy_dot]


### Fluid flow: summary

With all the assumptions so far, the mass conservation equations read
\begin{equation}
\begin{aligned}
  0 = & \phi \dot{S}_{\mathrm{water}}
  - \nabla \left( k \frac{k_{\mathrm{r\,water}}}{\mu_{\mathrm{water}}}\nabla P \right)\ , \\
  0 = & -\phi\dot{S}_{\mathrm{water}}
  -
  \nabla \left( k \frac{k_{\mathrm{r\,co2}}}{\mu_{\mathrm{co2}}}\nabla P \right) \ .
\end{aligned}
\end{equation}
The sum of these yields the total flow rate:
\begin{equation}
  Q(t) = -k \left(\frac{k_{\mathrm{r\,water}}}{\mu_{\mathrm{water}}} +
  \frac{k_{\mathrm{r\,co2}}}{\mu_{\mathrm{co2}}} \right) \nabla P \ .
\end{equation}
This total flow rate is dependent on time (controlled by the boundary
conditions) but satisfies $\nabla\cdot Q = 0$.

For later use, define the fractional flows
\begin{equation}
\label{frac.flows.defn.eqn}
\begin{aligned}
  Q_{\mathrm{water}} & =
  \frac{\frac{k_{\mathrm{r\,water}}}{\mu_{\mathrm{water}}}}{\frac{k_{\mathrm{r\,water}}}{\mu_{\mathrm{water}}}
    + \frac{k_{\mathrm{r\,co2}}}{\mu_{\mathrm{co2}}}} \\
    Q_{\mathrm{co2}} & =
    \frac{\frac{k_{\mathrm{r\,co2}}}{\mu_{\mathrm{co2}}}}{\frac{k_{\mathrm{r\,water}}}{\mu_{\mathrm{water}}}
    + \frac{k_{\mathrm{r\,co2}}}{\mu_{\mathrm{co2}}}} \ .
\end{aligned}
\end{equation}

### Heat flow: heat energy

In PorousFlow, the heat energy per unit volume is
\begin{equation}
\label{en.per.vol.eqn}
\mathcal{E} = (1-\phi)\rho_{R}C_{R}T +
\phi\sum_{\beta}S_{\beta}\rho_{\beta}\mathcal{E}_{\beta}
\end{equation}

LaForce et al. make the following assumptions

- The rock grain density, $\rho_{R}$, is constant.

- The specific heat capacity of the rock grains, $C_{R}$, is constant.

!listing modules/porous_flow/examples/thm_example/2D.i start=[internal_energy_reservoir] end=[elasticity_tensor]

- The internal energy of the liquid phase is $\mathcal{E}_{\mathrm{liquid}} = C_{\mathrm{water}}T$, where $C_{\mathrm{water}}$ is constant and independent of all other parameters in the system.

- The internal energy of the gas phase is $\mathcal{E}_{\mathrm{gas}} = C_{\mathrm{co2}}T$, where $C_{\mathrm{co2}}$ is constant and independent of all other parameters of the system.

With these assumptions the heat energy density is
\begin{equation}
\mathcal{E} = (1-\phi)\rho_{R}C_{R}T + \phi
S_{\mathrm{water}}\rho_{\mathrm{water}}C_{\mathrm{water}}T + \phi(1 -
S_{\mathrm{water}})\rho_{\mathrm{co2}}C_{\mathrm{co2}}T \ .
\end{equation}

### Heat flow: flux

In PorousFlow, the heat flux is a sum of heat conduction and convection with the fluid:
\begin{equation}
\label{heat.flux.eqn}
\mathbf{F}^{T} = -\lambda \nabla T + \sum_{\beta}h_{\beta}\mathbf{F}_{\beta}
\ .
\end{equation}

LaForce et al. make the following assumptions

- The only non-zero component of the thermal conductivity is the $zz$ component.  It varies with water saturation
  \begin{equation}
    \lambda_{zz} = \lambda_{zz}^{0} + (\lambda_{zz}^{1} -
    \lambda_{zz}^{0})S_{\mathrm{water}} \ .
  \end{equation}
  The coefficients $\lambda_{zz}^{0}$ and $\lambda_{zz}^{1}$ are independent of time, fluid pressure, temperature and mechanical deformation, but they are dependent on rock type.

!listing modules/porous_flow/examples/thm_example/2D.i start=[thermal_conductivity_reservoir] end=[internal_energy_reservoir]

- The enthalpy of the liquid phase is $h_{\mathrm{liquid}} = C_{\mathrm{water}}T$.

- The enthalpy of the gas phase is $h_{\mathrm{gas}} = C_{\mathrm{co2}}T$.

With the assumptions made so far
\begin{equation}
  \mathbf{F}^{T} = -\lambda \nabla T + C_{\mathrm{water}}
  \rho_{\mathrm{water}}{\mathbf{Q}}_{\mathrm{water}} T  + C_{\mathrm{co2}}
  \rho_{\mathrm{co2}}{\mathbf{Q}}_{\mathrm{co2}} T \ ,
\end{equation}
where the fractional flows of [frac.flows.defn.eqn] have been employed.

### Heat flow: energy conservation

In PorousFlow, energy conservation for heat is described by the continuity
equation
\begin{equation}
0 = \frac{\partial\mathcal{E}}{\partial t} + \mathcal{E}\nabla\cdot{\mathbf
  v}_{s} + \nabla\cdot \mathbf{F}^{T} -
\nu
  (1-\phi)\sigma^{\mathrm{eff}}_{ij}\frac{\partial}{\partial
    t}\epsilon_{ij}^{\mathrm{plastic}}
 - q^{T}
\label{heat.cons.eqn}
\end{equation}

LaForce et al. make the following assumptions

- There is no plastic heating.

- There are no heat sources, save for on the boundary.

### Heat flow: summary

With the assumptions made so far, the heat equation reads
\begin{equation}
  0 = \frac{\partial}{\partial t}(\rho C T) - \nabla (\lambda \nabla T)
  + \nabla (\rho C \mathbf{Q} T)
  \label{simplified.heat.eqn}
\end{equation}
where the following two variables, $\rho C$ and $\rho C \mathbf{Q}$
have been defined:
\begin{equation}
  \label{defns.rhoc.rhocq.eqn}
\begin{aligned}
  \rho C & = (1 - \phi)\rho_{R}C_{R} + \phi
  S_{\mathrm{water}}\rho_{\mathrm{water}}C_{\mathrm{water}} + \phi (1
  - S_{\mathrm{water}}) \rho_{\mathrm{co2}}C_{\mathrm{co2}} \ , \\
  \rho C \mathbf{Q} & = \rho_{\mathrm{water}} C_{\mathrm{water}}
  {\mathbf{Q}}_{\mathrm{water}}  + \rho_{\mathrm{co2}} C_{\mathrm{co2}}
  {\mathbf{Q}}_{\mathrm{co2}} \ .
\end{aligned}
\end{equation}
[simplified.heat.eqn] and [defns.rhoc.rhocq.eqn] are exactly Eq. (A1) in [!citet](laforce2014a).

During the course of their analysis, LaForce et al. replace the
diffusive term, $\nabla\lambda\nabla T$ with a Lauwerier heat-loss
expression:
\begin{equation}
\nabla(\lambda\nabla T) \rightarrow -\tilde{\lambda}(T -
T_{\mathrm{ref}}) \ .
\label{eqn.lauwerier.replace.1}
\end{equation}
In terms of LaForce et al.'s variables, the new heat-loss coefficient
is
\begin{equation}
\tilde{\lambda} = U_{L}/h = \frac{\sqrt{K_{a}(\rho C)_{a}}}{h\sqrt{t}}
\ .
\label{eqn.lauwerier.replace.2}
\end{equation}
This is a particularly nice simplification because it means that the
overburden and underburden don't need to be considered any further in
the analysis.  It does mean, however, that the mechanical effects of
these surrounding rocks on the aquifer are not modelled by LaForce et al.

Therefore, the heat-flow `Kernels` are

!listing modules/porous_flow/examples/thm_example/2D.i start=[energy_dot] end=[grad_stress_r]

along with the rate of heat loss:

!listing modules/porous_flow/examples/thm_example/2D.i block=Functions



### Solid mechanics

Denote the effective stress tensor by $\sigma^{\mathrm{eff}}$.  In PorousFlow it is
defined by
\begin{equation}
\sigma^{\mathrm{eff}}_{ij} = \sigma^{\mathrm{tot}}_{ij} +
\alpha_{B}\delta_{ij}P_{f} \ .
\label{eff.stress.eqn}
\end{equation}

LaForce et al. assume

- The Biot coefficient, $\alpha_{B}$, is constant and independent of all other parameters in the theory.

- The effective fluid pressure is $P_{f}=P$

!listing modules/porous_flow/examples/thm_example/2D.i start=[eff_fluid_pressure] end=[vol_strain]

In PorousFlow, the solid-mechanical constitutive law reads
\begin{equation}
\sigma_{ij}^{\mathrm{eff}} = E_{ijkl}\left(\epsilon^{\mathrm{elastic}}_{kl} -
\delta_{kl}\alpha_{T}(T - T_{\mathrm{ref}})\right)\ .
\label{eqn.elasticity}
\end{equation}

LaForce et al. assume

- The drained elasticity tensor, $E_{ijkl}$, is isotropic, constant and independent of all other parameters in the theory.

!listing modules/porous_flow/examples/thm_example/2D.i start=[elasticity_tensor] end=[strain]

- There is no plasticity.

- The drained thermal expansion coefficient, $\alpha_{T}$, is constant and independent of all other parameters in the theory.

!listing modules/porous_flow/examples/thm_example/2D.i start=[strain] end=[eff_fluid_pressure]


In PorousFlow, conservation of momentum reads
\begin{equation}
\rho_{\mathrm{mat}}\frac{\partial v_{s}^{j}}{\partial t} =
\nabla_{i}\sigma_{ij}^{\mathrm{tot}} + \rho_{\mathrm{mat}}b_{j} =
\nabla_{i}\sigma_{ij}^{\mathrm{eff}} - \alpha_{B}\nabla_{j} P_{f} + \rho_{\mathrm{mat}}b_{j} \ .
\label{cons.mom.eqn}
\end{equation}

LaForce et al. assume

- The acceleration of the solid skeleton, $\dot{v}_{s}$, is zero.

- There are no body forces, $b = 0$, except for on the boundary.


### Further assumptions and implications

LaForce et al. make the following assumptions:

- There is no displacement in the $z$ direction, $u_{z} = 0$.  The solid-mechanics `Kernels` are therefore

!listing modules/porous_flow/examples/thm_example/2D.i start=[grad_stress_r] end=[]


- The reservoir is free to contract and expand radially without being hindered by the impermeable seals.

There is no dependence on the axial coordinates, $z$, because of the use of the Lauwerier heat-loss model ([eqn.lauwerier.replace.1]) that simulates axial heat conduction.  Therefore, all independent variables --- the water saturation $S_{\mathrm{water}}$, fluid porepressure $P$, temperature $T$, and radial displacement $u_{r}$ --- are dependent on radius $r$ only.

The `Variables` used in the PorousFlow input file are:

!listing modules/porous_flow/examples/thm_example/2D.i block=Variables


## Parameter values and initial conditions

!table id=table_params caption=Parameters and their numerical values used in the benchmark against LaForce et al.
| Symbol | Value | Physical description |
| --- | --- | --- |
| $\alpha_{B}$  |  1.0  |  Biot coefficient for reservoir and seals |
| $\alpha_{T}$  |  $5\times 10^{-6}\,$K  |  Drained linear thermal expansion coefficient of the reservoir and seals |
| $C_{\mathrm{reservoir}}$  |  1100$\,$J.kg$^{-1}$.K$^{-1}$  |  Specific heat capacity of reservoir rock grains |
| $C_{\mathrm{seal}}$  |  828.9$\,$J.kg$^{-1}$.K$^{-1}$  |  Specific heat capacity of seal rock grains |
| $C_{\mathrm{co2}}$  |  2920.5$\,$J.kg$^{-1}$.K$^{-1}$  |  Specific heat capacity of water |
| $C_{\mathrm{water}}$  |  4149$\,$J.kg$^{-1}$.K$^{-1}$  |  Specific heat capacity of water |
| $\phi_{\mathrm{reservoir}}$  |  0.2  |  Porosity of reservoir  |
| $\phi_{\mathrm{seal}}$  |  0.02  |  Porosity of seals  |
| $G$  |  6$\,$GPa  |  Shear modulus of the reservoir and seals |
| $h_{\mathrm{reservoir}}$  |  11$\,$m  |  Vertical height of the reservoir  |
| $I$  |  $5\times 10^{5}\,$Tonne.year$^{-1}$  |  Injection rate of CO$_{2}$.  |
| $K_{\mathrm{drained}}$  |  8$\,$GPa  |  Drained bulk modulus of the reservoir and seals |
| $K_{\mathrm{water}}$  |  $2.27\times 10^{5}\,$GPa  |  Bulk density of water |
| $K_{\mathrm{co2}}$  |  $2.27\times 10^{5}\,$GPa  |  Bulk density of CO$_{2}$ |
| $k_{\mathrm{reservoir}}$  |  $2\times 10^{-12}\,$m$^{2}$  |  horizontal permeability components of the reservoir (there is zero vertical permeability)  |
| $k_{\mathrm{seal}}$  |  0  |  permeability of seal  |
| $\lambda_{\mathrm{reservoir}}^{0}$  | 1.32$\,$J.s$^{-1}$.m$^{-1}$.K$^{-1}$  |  Vertical thermal conductivity of reservoir at zero water saturation  |
| $\lambda_{\mathrm{reservoir}}^{1}$  | 3.083$\,$J.s$^{-1}$.m$^{-1}$.K$^{-1}$  |  Vertical thermal conductivity of reservoir at full water saturation  |
| $\lambda_{\mathrm{seal}}^{0}$  | 1.6$\,$J.s$^{-1}$.m$^{-1}$.K$^{-1}$  |  Vertical thermal conductivity of seal at zero water saturation  |
| $\lambda_{\mathrm{seal}}^{1}$  | 4.31$\,$J.s$^{-1}$.m$^{-1}$.K$^{-1}$  |  Vertical thermal conductivity of seal at full water saturation  |
| $\mu_{\mathrm{water}}$  |  $3.394\times 10^{-4}\,$Pa.s  |  Viscosity of water  |
| $\mu_{\mathrm{co2}}$  |  $3.93\times 10^{-5}\,$Pa.s  |  Viscosity of CO$_{2}$  |
| $\nu_{\mathrm{drained}}$  |  0.2  |  Drained Poisson's ratio of the reservoir and seals  |
| $P_{0}$  |  18.3$\,$MPa  |  Initial porepressure  |
| $r_{\mathrm{bh}}$  |  0.1$\,$m  |  Borehole radius  |
| $r_{\mathrm{max}}$  |  5$\,$km  |  Radial size of the model  |
| $\rho_{\mathrm{R\,reservoir}}$  |  2350$\,$kg.m$^{-3}$  |  Density of reservoir rock grains  |
| $\rho_{\mathrm{R\,seal}}$  |  2773.4$\,$kg.m$^{-3}$  |  Density of seal rock grains  |
| $\rho_{\mathrm{water}}$  |  970$\,$kg.m$^{-3}$  |  Density of water  |
| $\rho_{\mathrm{co2}}$  |  516.48$\,$kg.m$^{-3}$  |  Density of CO$_{2}$  |
| $S_{\mathrm{water\,0}}$  |  1.0  |  Initial water saturation  |
| $S_{\mathrm{co2}}^{\mathrm{res}}$  |  0.205  |  Residual saturation of CO$_{2}$  |
| $S_{\mathrm{water}}^{\mathrm{res}}$  |  0.2  |  Residual saturation of water  |
| $\sigma_{\mathrm{hor}}^{\mathrm{eff\,0}}$  |  -12.8$\,$MPa  |  Initial horizontal effective stress in reservoir and seals  |
| $\sigma_{\mathrm{ver}}^{\mathrm{eff\,0}}$  |  -51.3$\,$MPa  |  Initial vertical effective stress in reservoir and seals  |
| $T_{0}$  |  358$\,$K  |  Initial reservoir and seal temperature  |
| $T_{\mathrm{injection}}$  |  294$\,$K  |  Injection temperature of CO$_{2}$  |
| $T_{\mathrm{ref}}$  |  358$\,$K  |  Reference temperature for thermal strains and Lauwerier term |
| $t_{\mathrm{end}}$  |  5$\,$years  |  End-time for the simulation  |

These parameter values may be found through the PorousFlow input file.  For instance:

!listing modules/porous_flow/examples/thm_example/2D.i block=GlobalParams

and the fluid properties are

!listing modules/porous_flow/examples/thm_example/2D.i block=FluidProperties

Note the initial stress is *effective stress* and the horizontal initial stress occupies the first slot, while the vertical stress occupies the central slot

!listing modules/porous_flow/examples/thm_example/2D.i start=[ini_strain] end=[thermal_contribution]

The seals do not actually have to be considered in the model, because of LaForce et al.'s assumptions.

## Boundary conditions

LaForce et al. assume the following boundary conditions.

- The porepressure at the outer boundary is
  \begin{equation}
    P(r_{\mathrm{max}}) = P_{0} \ .
  \end{equation}

!listing modules/porous_flow/examples/thm_example/2D.i start=[outer_pressure_fixed] end=[outer_temp_fixed]

- The temperature at the outer boundary is
  \begin{equation}
    T(r_{\mathrm{max}}) = T_{0} \ .
  \end{equation}

!listing modules/porous_flow/examples/thm_example/2D.i start=[outer_temp_fixed] end=[fixed_outer_r]

- All displacements at the outer boundary are zero
  \begin{equation}
    u(r_{\mathrm{max}}) = 0 \ .
  \end{equation}

!listing modules/porous_flow/examples/thm_example/2D.i start=[fixed_outer_r] end=[co2_injection]

- The injection is at a constant rate of $5\times 10^5\,$T.year$^{-1}$ at a constant temperature of 294$\,$K.  In the PorousFlow input file, the fluid injection is implemented as a [PorousFlowSink](boundaries.md), which slowly ramps up to its full value during the initial 100$\,$s in order to help convergence, and a preset `DirichletBC`:

!listing modules/porous_flow/examples/thm_example/2D.i start=[co2_injection] end=[cavity_pressure_x]

- The *total* radial compressive stress at the well is equal to the porepressure in the well.  This is dictated by the dynamics of the problem: since the injection rate is constant, the porepressure will increase with time.
  \begin{equation}
    \sigma_{rr}^{\mathrm{tot}}(r_{\mathrm{bh}}) = -P \ .
  \end{equation}

This is implemented in a slightly convoluted way in the MOOSE input file.  First, a `Postprocessor` records the value of porepressure at the beginning of each time step:

!listing modules/porous_flow/examples/thm_example/2D.i block=Postprocessors

Then a [PressureBC](bcs/Pressure.md) applies this *total* stress (physically, a mechanical pushing force) at the borehole wall:

!listing modules/porous_flow/examples/thm_example/2D.i start=[cavity_pressure_x] end=[]

The `component` is zero because this is an axially-symmetric (2D RZ) problem.  The value of the total pressure should be equal to the variable `pwater`, but because the `p_bh` is recorded at the start of each time step, this boundary condition actually lags by one time step.  Because porepressure `pwater` does not change significantly over each time step, this leads to minimal error.

## Results

!media media/porous_flow/2D_thm_compare_porepressure_fig.png caption=Comparison between the PorousFlow result and the analytic expression derived by LaForce et al. for the porepressure (Eq. (12) in [!citet](laforce2014b)) id=porepressure_cf_fig

!media media/porous_flow/2D_thm_compare_temperature_fig.png caption=Comparison between the PorousFlow result and the analytic expression derived by LaForce et al. for the temperature (Eq. (20) in [!citet](laforce2014a)) id=temperature_cf_fig

!media media/porous_flow/2D_thm_compare_displacement_fig.png caption=Comparison between the PorousFlow result and the analytic expression derived by LaForce et al. for the radial displacement (Eq. (33) in [!citet](laforce2014b)) id=displacement_cf_fig

!media media/porous_flow/2D_thm_compare_sg_fig.png caption=Comparison between the PorousFlow result and the analytic expression derived by LaForce et al. for the gas saturation (from Eq. (16) in [!citet](laforce2014a)) id=sg_cf_fig

!media media/porous_flow/2D_thm_compare_seff_rr_fig.png caption=Comparison between the PorousFlow result and the analytic expression derived by LaForce et al. for the effective radial stress (Eq. (A3) in [!citet](laforce2014b)).  The small discrepancy at the borehole wall is due to the finite resolution of the MOOSE model, where stresses are evaluated at finite-element centroids instead of at nodes. id=seff_rr_cf_fig

!media media/porous_flow/2D_thm_compare_seff_tt_fig.png caption=Comparison between the PorousFlow result and the analytic expression derived by LaForce et al. for the effective hoop stress (Eq. (A3) in [!citet](laforce2014b)).  The small discrepancy at the borehole wall is due to the finite resolution of the MOOSE model, where stresses are evaluated at finite-element centroids instead of at nodes. id=seff_tt_cf_fig

## Acknowledgement

The authors wish to acknowledge financial assistance provided through Australian National Low Emissions Coal Research and Development [ANLEC R&D](https://anlecrd.com.au).  ANLEC R&D is supported by Australian Coal Association Low Emissions Technology Limited and the Australian Government through the Clean Energy Initiative.

## A chemically-reactive, elastic reservoir

[Another page](thmc_example.md) describes how the model of this page may be extended to include geochemical reactions in the reservoir.


!bibtex bibliography
