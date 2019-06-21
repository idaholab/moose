# Governing equations

The equations for heat flow, fluid flow, solid mechanics, and chemical reactions
are defined here.  Notation used in these equations is summarised in the
[nomenclature](/porous_flow/nomenclature.md). The Lagrangian coordinate system
is used since it moves with the mesh.

## Fluid flow

Mass conservation for fluid species $\kappa$ is described by the continuity
equation
\begin{equation}
\label{eq:mass_cons_sp}
0 = \frac{\partial M^{\kappa}}{\partial t} + M^{\kappa}\nabla\cdot{\mathbf
  v}_{s} + \nabla\cdot \mathbf{F}^{\kappa} + \Lambda M^{\kappa} - \phi I_{\mathrm{chem}} - q^{\kappa} \ .
\end{equation}
Here $M$ is the mass of fluid per bulk volume (measured in
kg.m$^{-3}$), $\mathbf{v}_{s}$ is the velocity of the porous solid
skeleton (measured in m.s$^{-1}$), $\mathbf{F}$ is the flux (a vector,
measured kg.s$^{-1}$.m$^{-2}$), $\Lambda$ is a radioactive decay rate, $\phi I_{\mathrm{chem}}$ represents chemical precipitation or dissolution
and $q$ is a source (measured in kg.m$^{-3}$.s$^{-1}$).

The coupling to the solid mechanics is via the $M\nabla\cdot \mathbf{v}_{s}$
term, as well as via changes in porosity and permeability.  Coupling to heat
flow and chemical reactions is via the equations of state used within the terms
of [eq:mass_cons_sp], as well as the source term $q^{\kappa}$.

The species are parameterised by $\kappa = 1,\ldots$.  For example,
$\kappa$ might parameterise water, air, H$_2$, a solute, and so
on.  $\kappa$ parameterises things which cannot be decomposed into
other species, but can change phase.  For instance, sometimes it might
be appropriate to consider air as a single species (say $\kappa=1$),
while other times it might be appropriate to consider it to be a
mixture of nitrogen and oxygen ($\kappa=1$ and $\kappa=2$).  To
model chemical precipitation, a solid phase, which is distinct from
the porous skeleton, may also be used (its relative permeability will
be zero: see below).

## Mass density

The mass of species $\kappa$ per volume of rock is written as a sum
over all phases present in the system:
\begin{equation}
\label{eq:msph}
M^{\kappa} =
\phi\sum_{\beta}S_{\beta}\rho_{\beta}\chi_{\beta}^{\kappa}
+ (1 - \phi)C^{\kappa}
\end{equation}
The solid's porosity is $\phi$.  $S_{\beta}$ is the saturation of
phase $\beta$ (solid, liquid, gas, NAPL).  $\rho_{\beta}$ is the density of
phase $\beta$.  $\chi_{\beta}^{\kappa}$ is the mass fraction
of component $\kappa$ present in phase $\beta$.  The final term
represents fluid absorption into the porous-rock skeleton:
$C^{\kappa}$ is the mass of absorbed species per volume of solid
rock-grain material.

The density $\rho_{\beta}$ is typically a function of pressure and temperature,
but may also depend on mass fraction of individual components, as described by
the equation of state used.

The saturation and mass fractions must obey
\begin{equation}
\begin{aligned}
\sum_{\beta}S_{\beta} & =  1 \ , \\
\sum_{\kappa}\chi_{\beta}^{\kappa} & =  1 \ \ \ \forall
\beta \ .
\end{aligned}
\end{equation}

### Flux

The flux is a sum of advective flux and diffusive-and-dispersive flux:
\begin{equation}
\label{eq:adv_diff_disp}
\mathbf{F}^{\kappa} =
\sum_{\beta}\chi_{\beta}^{\kappa}\mathbf{F}_{\beta}^{\mathrm{advective}}
+ \mathbf{F}^{\kappa}_{\mathrm{diffusion+dispersion}} \ .
\end{equation}

#### Advection

Advective flux is governed by Darcy's law.  Each phase is assumed to
obey Darcy's law.  Each phase has its own density, $\rho_{\beta}$,
relative permeability $k_{\mathrm{r,}\beta}$, viscosity
$\mu_{\beta}$, and pressure $P_{\beta}$.  These may all be nonlinear
functions of the independent variables.  With them, we can form the
advective Darcy flux:
\begin{equation}
\label{eq:darcy}
\mathbf{F}_{\beta}^{\mathrm{advective}} = \rho_{\beta}\mathbf{v}_{\beta} =
-\rho_{\beta}\frac{k\,k_{\mathrm{r,}\beta}}{\mu_{\beta}}(\nabla
P_{\beta} - \rho_{\beta} \mathbf{g})
\end{equation}
In this equation $\mathbf{v}_{\beta}$ is the Darcy velocity (volume
flux, measured in m.s$^{-1}$) in phase $\beta$.  It is used below in the diffusive-and-dispersion flux
too.

The absolute permeability is denoted by $k$ and it is a tensor.  The
relative permeability of phase $\beta$ is
denoted by $k_{\mathrm{r,}\beta}$.  It is always a function of the
saturation(s), but with Klinkenberg effects, it may
  also be a function of the gas pressure.  Relative permeability can
  also be hysteretic, so that it depends on the history of
  saturation.

!alert note
In reality, relative permeability is actually a tensor
(for example it's usually different in lateral and vertical
directions) but is most often treated as a scalar, since it's hard to
get parameters for the tensorial case.  In the PorousFlow module it is
treated as a scalar

In some cirumstances $K_{ij}\nabla_{j}T$ is added to the above Darcy flux to
model thermo-osmosis (with some $K_{ij}$ tensor parameterising its strength),
i.e. a gradient of temperature induces fluid flow.

Also note:

- The pressure in phase $\beta$ is $P_{\beta} = P + P_{c,\beta}$ where $P$ is the reference pressure (usually taken to be the gas phase), and $P_{c,\beta}$ is the capillary pressure (nonpositive if the reference phase is the gas phase).  The capillary pressure is often a function of saturation, and various forms have been coded into the PorousFlow module: see [capillary pressure](/porous_flow/capillary_pressure.md). *The capillary pressure relationship used in a model can have a great bearing on both the speed of convergence and the results obtained.* The capillary pressure relationship can also be hysteretic, in that it can depend on the history of the saturation.

- The pressure in a gas phase is the sum of the gas partial pressures: $P_{\mathrm{gas}} = \sum_{\kappa}P_{\mathrm{gas}}^{\kappa}$.  (The partial pressure of a gaseous species is $P_{\mathrm{gas}}^{\kappa} = P_{\mathrm{gas}}N^{\kappa}/N$ where $N$ is the number of molecules. This is Dalton's law.) The partial pressure concept is reasonable for dilute gases, but is less useful for dense gases.

- The mass-fraction of a species in the aqueous phase is often computed using Henry's law: $\chi_{\mathrm{aqueous}}^{\kappa} = P_{\mathrm{gas}}^{\kappa}/H_{\kappa}$ where $H_{\kappa}$ is the Henry coefficient.  Occasionally this law is not accurate enough, and there are more complicated alternatives.

- When a liquid and a gaseous phase exist, the simplest equation for vapour pressure is $P_{\mathrm{vapour}} = P_{\mathrm{sat}}(T)$ which is just the saturated vapour pressure of the liquid phase.  This can set the temperature $T$ from the gaseous pressure, or vice-versa, depending on the choice of independent variables.  A more complicated alternative is the Kelvin equation for vapour pressure that takes into account vapour pressure lowering due to capillarity and phase adsorption affects $P_{\mathrm{vapour}} = \exp\left( \frac{M_{w}P_{c,l}(S_{l})}{\rho_{l}R(T+273.15)} \right) P_{sat}(T)$. Here $M_{w}$ is the molecular weight of water; $P_{c,l}=P_{c,l}(S_{l})\leq 0$ is the capillary pressure --- the difference between aqueous (liquid water) and gas phase pressures --- a function of $S_{l}$; $\rho_{l}$ is the aqueous (liquid water) density; $R$ is the universal gas constant; $T$ is the temperature; $P_{sat}$ is the saturated vapour pressure of the bulk aqueous (liquid water) phase.

#### Diffusion and hydrodynamic dispersion

Diffusion and dispersion are proportional to the gradient of
$\chi_{\beta}^{\kappa}$.  A detailed discussion of multiphase diffusion and
dispersion is contained in Appendix D of the TOUGH2 manual [!citep](Pruess1999).
Here we use the common expression
\begin{equation}
\label{eq:diff_disp}
\mathbf{F}^{\kappa}_{\mathrm{diffusion+dispersion}} =
-\sum_{\beta}\rho_{\beta}{\mathcal{D}}_{\beta}^{\kappa}\nabla
\chi_{\beta}^{\kappa}
\end{equation}
Note $\mathbf{F}$ is a
vector, ${\mathcal{D}}$ is a 2-tensor and $\nabla$ a vector.  The hydrodynamic
dispersion tensor is
\begin{equation}
\label{eq:hydro_disp}
{\mathcal{D}}_{\beta}^{\kappa} =
D_{\beta,T}^{\kappa}{\mathcal{I}} + \frac{D_{\beta,L}^{\kappa} - D_{\beta,
T}^{\kappa}}{\mathbf{v}_{\beta}^{2}}\mathbf{v}_{\beta}\mathbf{v}_{\beta} \ ,
\end{equation}
where
\begin{equation}
\begin{aligned}
D_{\beta,L}^{\kappa} & = \phi \tau_0 \tau_{\beta} d_{\beta}^{\kappa} + \alpha_{\beta,L} \left| \mathbf{v} \right|_{\beta}, \\
D_{\beta,T}^{\kappa} & = \phi \tau_0 \tau_{\beta} d_{\beta}^{\kappa} + \alpha_{\beta,T}\left| \mathbf{v} \right|_{\beta}.
\end{aligned}
\end{equation}

These are called the longitudinal and transverse dispersion coefficients.
$d_{\beta}^{\kappa}$ is the molecular diffusion coefficient for component
$\kappa$ in phase $\beta$. $\tau_{0}\tau_{\beta}$ is the tortuosity which
includes a porous medium dependent factor $\tau_{0}$ and a coefficient
$\tau_{\beta} = \tau_{\beta}(S_{\beta})$, and $\alpha_{L}$ and $\alpha_{T}$ are
the longitudinal and transverse dispersivities.  It is common to set the
hydrodynamic dispersion to zero by setting $\alpha_{\beta, T} = 0 =
\alpha_{\beta, L}$.

!alert note
Multiple definitions of tortuosity appear in the literature. In
PorousFlow, tortuosity is defined as the ratio of the shortest path to
the effective path, so that $0 < \tau \leq 1$.

## Heat flow

Energy conservation for heat is described by the continuity
equation
\begin{equation}
\label{eq:heat_cons}
0 = \frac{\partial\mathcal{E}}{\partial t} + \mathcal{E}\nabla\cdot{\mathbf
  v}_{s} + \nabla\cdot \mathbf{F}^{T} -
\nu
  (1-\phi)\sigma^{\mathrm{eff}}_{ij}\frac{\partial}{\partial
    t}\epsilon_{ij}^{\mathrm{plastic}}
 - q^{T}
\end{equation}
Here $\mathcal{E}$ is the heat energy per unit volume in the rock-fluid
system, ${\mathbf v}_{s}$ is velocity of the porous solid skeleton,
$\mathbf{F}^{T}$ is the heat flux, $\nu$ describes the ratio of
plastic-deformation energy that gets transferred to heat energy,
$\sigma^{\mathrm{eff}}_{ij}$ is the effective stress (see
[eq:eff_stress]),
$\epsilon_{ij}^{\mathrm{plastic}}$ is the plastic strain, and $q^{T}$
is a heat source.

The coupling to the solid mechanics is via the $\mathcal{E}\nabla\cdot{\mathbf
v}_{s}$ term, the $\nu
(1-\phi)\sigma^{\mathrm{eff}}_{ij}\frac{\partial}{\partial
t}\epsilon_{ij}^{\mathrm{plastic}}$ term, as well as via changes in porosity and
permeability described in [porosity](/porous_flow/porosity.md) and
[permeability](/porous_flow/permeability.md). Coupling to the fluid flow and
chemical reactions is via the equations of state used within the terms of [eq:heat_cons], as well as the source term $q^{T}$.  Joule-Thompson
effects (See for instance Eq. (1) of [!cite](mathias2010)) may be included via
the fluid properties.

Here it is assumed the liquids and solid are in local thermal equilibrium i.e.
there is a single local temperature in all phases.   If this doesn't hold, one
is also normally in the high-flow regime where the flow is non-Darcy as well.

Sometimes a term is added that captures the thermal power due to volumetric
expansion of the fluid, $K\beta T\nabla\cdot\mathbf{v}$, where $K$ is the bulk
modulus of the fluid or solid, $\beta$ is the thermal expansion coefficient, and
$\mathbf{v}$ is the Darcy velocity. This is not included in the Porous Flow
module currently.

### Energy density: $\mathcal{E}$

The heat energy per unit volume is
\begin{equation}
\label{eq:en_per_vol}
\mathcal{E} = (1-\phi)\rho_{R}C_{R}T +
\phi\sum_{\beta}S_{\beta}\rho_{\beta}\mathcal{E}_{\beta} +
\sum_{\kappa}(1 -
  \phi)\rho^{R}\mathcal{E}_{\mathrm{abs\,}\kappa}A^{\kappa}
\end{equation}
The notation is: porosity $\phi$; grain density $\rho_{R}$; specific
heat capacity of rock $C_{R}$; temperature $T$; saturation of phase
$S_{\beta}$ (solid, liquid, gas, NAPL); density of phase $\rho_{\beta}$;
internal energy in phase $\mathcal{E}_{\beta}$; internal energy of
absorbed species $\mathcal{E}_{\mathrm{abs\,}\kappa}$.


### Heat flux: $\mathbf{F}^{T}$

The heat flux is a sum of heat conduction and convection with the
fluid:
\begin{equation}
\label{eq:heat_flux}
\mathbf{F}^{T} = -\lambda \nabla T + \sum_{\beta}h_{\beta}\mathbf{F}_{\beta}
\ .
\end{equation}
Here $\lambda$ is the tensorial thermal conductivity of the rock-fluid
system, which is a function of the thermal conductivities of
rock and fluid phases.  Usually $\lambda$ will be diagonal but in
anisotropic porous materials it may not be.
The specificy enthalpy of phase $\beta$ is denoted by $h_{\beta}$, and
$\mathbf{F}_{\beta}$ is the advective Darcy flux.

## Solid mechanics

Most of the solid mechanics used by the Porous Flow module is handled by the
[Tensor Mechanics](/tensor_mechanics/index.md) module.  This section provides a
brief overview, concentrating on the aspects that differ from pure solid
mechanics.

Denote the total stress tensor by $\sigma^{\mathrm{tot}}$.  An externally
applied mechanical force will create a nonzero $\sigma^{\mathrm{tot}}$, and
conversely, resolving $\sigma^{\mathrm{tot}}$ into forces yields the forces on
nodes in the finite-element mesh.

Denote the effective stress tensor by $\sigma^{\mathrm{eff}}$.  It is
defined by
\begin{equation}
\label{eq:eff_stress}
\sigma^{\mathrm{eff}}_{ij} = \sigma^{\mathrm{tot}}_{ij} +
\alpha_{B}\delta_{ij}P_{f} \ .
\end{equation}
The notation is as follows.

- $P_{f}$ is a measure of porepressure.  In single-phase,
  fully-saturated situations it is traditional to use $P_{f} =
  P_{\beta}$.  However, for multi-phase situations
  $P_{f}=\sum_{\beta}S_{\beta}P_{\beta}$ is also used. Yet other
  expressions involve Bishop's parameter.
- $\alpha_{B}$ is the *Biot coefficient*.  This obeys
  $0\leq\alpha_{B}\leq 1$. For a multi-phase system, the Biot
  coefficient is often chosen to be $\alpha_{B}=1$. The Biot
  coefficient is interpreted physically by the following. If, by
  pumping fluid into a porous material, the $P_{f}$ porepressure is
  increased by $\Delta P_{f}$, and at the same time a mechanical
  external force applies an incremental pressure equaling
  $\alpha_{B}\Delta P_{f}$, then the volume of the porous solid
  remains static. (During this process, the porevolume and porosity
  will change, however, as quantified in [porosity](/porous_flow/porosity.md)).


It is assumed that the elastic constitutive law reads
\begin{equation}
\label{eq:elasticity}
\sigma_{ij}^{\mathrm{eff}} = E_{ijkl}(\epsilon^{\mathrm{elastic}}_{kl} -
\delta_{kl}\alpha_{T}T)\ ,
\end{equation}
with $\alpha_{T}$ being the thermal expansion coefficient of the
drained porous skeleton, and $\epsilon_{kl} = (\nabla_{k}u_{l} +
\nabla_{l}u_{k})/2$ being the usual total strain tensor ($u$ is the
deformation of the porous solid), which can be split into the elastic
and plastic parts, $\epsilon = \epsilon^{\mathrm{elastic}} +
\epsilon^{\mathrm{plastic}}$, and $E_{ijkl}$ being the elasticity
tensor (the so-called *drained* version).  The generalisation to
large strain is obvious.  The inverse of the constitutive law is
\begin{equation}
\epsilon^{\mathrm{elastic}}_{ij} - \delta_{kl}\alpha_{T}T = C_{ijkl}\sigma_{ij}^{\mathrm{eff}} \ ,
\end{equation}
with $C$ being the compliance tensor.

It is assumed that the conservation of momentum is
\begin{equation}
\label{eq:cons_mom}
\rho_{\mathrm{mat}}\frac{\partial v_{s}^{j}}{\partial t} =
\nabla_{i}\sigma_{ij}^{\mathrm{tot}} + \rho_{\mathrm{mat}}b_{j} =
\nabla_{i}\sigma_{ij}^{\mathrm{eff}} - \alpha_{B}\nabla_{j} P_{f} + \rho_{\mathrm{mat}}b_{j} \ ,
\end{equation}
where ${\mathbf{v}}_{s} = \partial{\mathbf u}/\partial t$ is the
velocity of the solid skeleton, $\rho_{\mathrm{mat}}$ is the
mass-density of the material (this is the *undrained* density:
$\rho_{\mathrm{mat}} = (1 - \phi)\rho^{R} +
\phi\sum_{\beta}S_{\beta}\rho_{\beta}$), and $b_{j}$ are the
components of the external force density (for example, the
gravitational acceleration).  Here  any terms
of $O(v^{2})$ have been explicitly ignored , and it's been assumed that to this order the velocity of
each phase is identical to the velocity of the solid skeleton
(otherwise there are terms involving $\partial \mathbf{F}/\partial t$ on
the left-hand side).

It is assumed that the *effective stress not the total stress* enters into the
consitutive law (as above), and the plasticity, and the insitu stresses, and
almost everywhere else.  One exception is specifying Neumann boundary conditions
for the displacements where the total stresses are being specified, as can be
seen from [eq:cons_mom].  Therefore, MOOSE will use effective stress,
and not total stress, internally.  If one needs to input or output total stress,
one must subtract $\alpha_{B}\nabla_{j} P_{f}$ from MOOSE's stress.


## Chemical reactions

### Aqueous chemistry

Aqueous equilibrium chemistry and precipitation/dissolution kinetic chemistry have been implemented in the same way as the [chemical reactions module](/chemical_reactions/index.md).

### Adsorption and desorption

The fluid mass [eq:msph] contains
contributions from adsorped species: $C^{\kappa}$ is the mass of
absorped species per volume of solid rock-grain material.  Its
dynamics involves no advective flux terms, as PorousFlow assumes that
the adsorped species are trapped in the solid matrix.  The governing
equation is
\begin{equation}
0 = \frac{\partial}{\partial t}(1 - \phi)C^{\kappa} + (1 -
\phi)C^{\kappa}\nabla\cdot{\mathbf{v}}_{s} + L^{\kappa} \ .
\end{equation}
The $(1-\phi)$ terms account for the porosity of the solid skeleton
(so $(1-\phi)C^{\kappa}$ is the mass of adsorped species per volume
of porous skeleton), the coupling to the solid mechanics is via the
second term, and $L^{\kappa}$ governs the adsorption-desorption
process.

Currently, PorousFlow assumes that $C^{\kappa}$ is a primary MOOSE variable
(that is, defined in the `Variables` block of the MOOSE input file).  This is in
contrast to the remainder of PorousFlow where the primary variables can be
arbitrary (so that variable switching and persistent variables can be used in
difficult problems).  Usually $C^{\kappa}$ will have `family=MONOMIAL` and
`order=CONSTANT`.

PorousFlow assumes that $L$ follows the Langmuir form:
\begin{equation}
L = \frac{1}{\tau_{L}}\left(C - \frac{\rho_{L}P}{P_{L}+P} \right)
\end{equation}
Here $L$ is the mass-density rate (kg.m$^{-3}$.s$^{-1}$) of material
moving from the adsorped state to the porespace.  The notation is as
follows.

- $\rho_{L}$ is the so-called Langmuir density, which is
  adsorped-gas-mass per volume of solid rock-grain material (kg.m$^{-3}$).  In
  terms of the often used Langmuir volume, $V_{L}$, it is
  $\rho_{L}=V_{L}\times$ gas density at STP.
- $P_{L}$ is the Langmuir pressure (Pa)
- $P$ is the relevant phase pressure (which will be the pressure
  of the gas phase).  Currently, PorousFlow assumes that $P$ is a
  primary MOOSE variable (that is, defined in the `Variables`
  block of the MOOSE input file).  This is in contrast to the
  remainder of PorousFlow where the primary variables can be
  arbitrary.
- $\tau_{L}$ is the time constant.  Two time constants may be
  defined: one for desorption, and one for adsorption, so

\begin{equation}
\tau_{L} = \left\{
\begin{aligned}
\tau_{L,\ \mathrm{desorption}} & \textrm{ if }
C>\frac{\rho_{L}P}{P_{L}+P} \\
\tau_{L,\ \mathrm{adsorption}} & \textrm{ otherwise}
\end{aligned}
\right.
\end{equation}
A mollifier may also be defined to mollify the step-change between
desorption and adsorption.

## Implementation

Each part of the governing equations above are implemented in separate
`Kernels`. Simulations can then include any or all of the possible physics
available in the Porous Flow module by simply adding extra `Kernels` to the
input file.

!table id=kernels caption=Available `Kernels`
| Equation | Kernel |
| --- | --- |
| $\frac{\partial}{\partial t}\left(\phi\sum_{\beta}S_{\beta}\rho_{\beta}\chi_{\beta}^{\kappa}\right)$ | [`PorousFlowMassTimeDerivative`](PorousFlowMassTimeDerivative.md) |
| $(\rho)(\dot{P}/M - A\dot{T} + \alpha_{B}\dot{\epsilon}_{v})$ | [`PorousFlowFullySaturatedMassTimeDerivative`](PorousFlowFullySaturatedMassTimeDerivative.md) |
| $\frac{\partial}{\partial t}\left((1 - \phi)C^{\kappa}\right)$ | [`PorousFlowDesorpedMassTimeDerivative`](PorousFlowDesorpedMassTimeDerivative.md) |
| $-\nabla\cdot \sum_{\beta}\chi_{\beta}^{\kappa} \rho_{\beta}\frac{k\,k_{\mathrm{r,}\beta}}{\mu_{\beta}}(\nabla P_{\beta} - \rho_{\beta} \mathbf{g})$ | [`PorousFlowAdvectiveFlux`](PorousFlowAdvectiveFlux.md) |
| $-\nabla\cdot \sum_{\beta}\chi_{\beta}^{\kappa} \rho_{\beta}\frac{k\,k_{\mathrm{r,}\beta}}{\mu_{\beta}}(\nabla P_{\beta} - \rho_{\beta} \mathbf{g})$ | [`PorousFlowFluxLimitedTVDAdvection`](PorousFlowFluxLimitedTVDAdvection.md) |
| $-\nabla\cdot \sum_{\beta}\rho_{\beta}{\mathcal{D}}_{\beta}^{\kappa}\nabla \chi_{\beta}^{\kappa}$ | [`PorousFlowDispersiveFlux`](PorousFlowDispersiveFlux.md) |
| $-\nabla\cdot ((\rho)k(\nabla P - \rho \mathbf{g})/\mu)$ | [`PorousFlowFullySaturatedDarcyBase`](PorousFlowFullySaturatedDarcyBase.md) |
| $-\nabla\cdot (\rho\chi^{\kappa} k(\nabla P - \rho \mathbf{g})/\mu)$ | [`PorousFlowFullySaturatedDarcyFlow`](PorousFlowFullySaturatedDarcyFlow.md) |
| $\frac{\partial}{\partial t}\left((1-\phi)\rho_{R}C_{R}T + \phi\sum_{\beta}S_{\beta}\rho_{\beta}\mathcal{E}_{\beta}\right)$ | [`PorousFlowEnergyTimeDerivative`](PorousFlowEnergyTimeDerivative.md) |
| $-\nabla\cdot \left(\lambda \nabla T\right)$ | [`PorousFlowHeatConduction`](PorousFlowHeatConduction.md) |
| $-\nabla\cdot \sum_{\beta}h_{\beta} \rho_{\beta}\frac{k\,k_{\mathrm{r,}\beta}}{\mu_{\beta}}(\nabla P_{\beta} - \rho_{\beta} \mathbf{g})$ | [`PorousFlowHeatAdvection`](PorousFlowHeatAdvection.md) |
| $-\nabla\cdot \sum_{\beta}h_{\beta} \rho_{\beta}\frac{k\,k_{\mathrm{r,}\beta}}{\mu_{\beta}}(\nabla P_{\beta} - \rho_{\beta} \mathbf{g})$ | [`PorousFlowFluxLimitedTVDAdvection`](PorousFlowFluxLimitedTVDAdvection.md) |
| $-\nabla\cdot ((\rho)h k(\nabla P - \rho \mathbf{g})/\mu)$ | [`PorousFlowFullySaturatedHeatAdvection`](PorousFlowFullySaturatedHeatAdvection.md) |
| $\mathcal{E}\nabla\cdot\mathbf{v}_{s}$ | [`PorousFlowHeatVolumetricExpansion`](PorousFlowHeatVolumetricExpansion.md) |
| $-\nu (1-\phi)\sigma^{\mathrm{eff}}_{ij}\frac{\partial}{\partial t}\epsilon_{ij}^{\mathrm{plastic}}$ | [`PorousFlowPlasticHeatEnergy`](PorousFlowPlasticHeatEnergy.md) |
| $\phi\sum_{\beta}S_{\beta}\rho_{\beta}\chi_{\beta}^{\kappa}\nabla\cdot\mathbf{v}_{s}$ | [`PorousFlowMassVolumetricExpansion`](PorousFlowMassVolumetricExpansion.md) |
| $\left((1 - \phi)C^{\kappa}\right)\nabla\cdot\mathbf{v}_{s}$ | [`PorousFlowDesorpedMassVolumetricExpansion`](PorousFlowDesorpedMassVolumetricExpansion.md)|
| $\Lambda \phi\sum_{\beta}S_{\beta}\rho_{\beta}\chi_{\beta}^{\kappa}$ | [`PorousFlowMassRadioactiveDecay`](PorousFlowMassRadioactiveDecay.md) |
| $\phi I_{\mathrm{chem}}$ | [`PorousFlowPreDis`](PorousFlowPreDis.md) |
| $\nabla\cdot(\mathbf{v}_{\beta} u)$ | [`PorousFlowBasicAdvection`](PorousFlowBasicAdvection.md) |

!bibtex bibliography
