# Tidal impacts on porepressure

The porepressure in underground porous materials is influenced by external factors, such as earth tides, barometric pressure changes, and oceanic tides.  The response of porepressure to these factors can help quantify important hydrogeological parameters, such as the poroelastic constants and even the permeability.

This page illustrates how PorousFlow can be used to simulate these types of problems.  Only the simplest models are built, but PorousFlow can be used to simulate much more complicated situations, such as:

- realistic stratigraphy and hydrogeology (a mesh needs to be built, and parameters (permeability, capillary pressure function, etc) assigned to each domain);

- partially-saturated or multiphase flows (these capabilities of PorousFlow need to be activated);

- solid mechanical plasticity (strengths need to be assigned, and appropriate plastic models chosen);

- non-isothermal flows (heat advection and diffusion Kernels need to be added, along with appropriate material moduli);

- complicated model setups, such as a hierarchy of models at different scales that couple together: for instance, a continental model informing the boundary conditions on a regional model which sets the boundary conditions on a local model (MultiApps should be used).

A comprehensive introduction to tidal impacts is the online tutorial by Doan, Brodsky, Prioul and Signer [!citep](doanET2006).  This document describes the origin of earth tides, barometric fluctuations and oceanic tides, and also details how to compute them, the software available, etc.  It also details how to analyse observational data to obtain poroelastic parameters and permeability.

A comprehensive discussion of poroelasticity is [!citet](detournayET93). The "stress" and "total pressure" ($P$) in that document are total (not effective) stresses.  Another good hydrogeology reference is [!citet](wang2000).

## Properties used in the examples

All the PorousFlow examples mentioned in this page replicate standard poroelasticity exactly by using:

- a [PorousFlowBasicTHM](PorousFlowBasicTHM.md) Action with hydro-mechanical coupling only;

- `multiply_by_density = false`;

- [PorousFlowConstantBiotModulus](PorousFlowConstantBiotModulus.md).

It is not mandatory to use these in PorousFlow.  For instance, PorousFlow simulations usually do not assume a constant bulk modulus (compressibility) for the fluid(s), but without these assumptions the results will differ slightly from standard poroelasticity solutions.

[earth_tide_fullsat_params] lists the parameters used in the simulations.  In this table, fluid bulk modulus is the reciprocal of fluid compressibility ($C_{f}=1/K_{f}$).  Also, the porosity is the volume fraction of rock that is accessible by the fluid(s), which is often slightly smaller than the total porosity used by geologists.

!table id=earth_tide_fullsat_params caption=Parameters used in the MOOSE models
| Parameter | Value |
| --- | --- |
| Fluid bulk modulus ($K_{f}$) | 2 GPa |
| Porosity ($\phi$) | 0.1 |
| Biot coefficient ($\alpha_{B}$) | 0.6 |
| Drained bulk modulus of porous skeleton ($K$) | 10 GPa |
| Drained Poisson's ratio ($\nu$) | 0.25 |
| Isotropic permeability | $10^{-14}\,$m$^{2} = 10\,$mD |
| Fluid reference density | 1000 kg.m$^{-3}$ |
| Fluid constant viscosity | $10^{-3}\,$Pa.s |
| Average gravitational acceleration ($g$) | $10\,$m.s$^{-2}$ |

Using the values in [earth_tide_fullsat_params] along with [!citet](detournayET93) (viz Table 2, Equation (18) and Equation (51b)) the poroelastic constants may be derived, as shown in [params_derived]

!table id=params_derived caption=Derived poroelastic constants
| Parameter | Equation | Value |
| --- | --- | --- |
| Biot modulus | $\frac{1}{M} = \frac{\phi}{K_{f}} + \frac{(1 - \alpha_{B})(\alpha_{B} - \phi)}{K}$ | $M=14.3\,\mathrm{GPa}$ |
| Skempton coefficient | $B = \frac{\alpha_{B} K_{f}}{(\alpha_{B} - \phi(1-\alpha_{B}))K_{f} + \phi K}$ | $0.56604$ |
| Undrained bulk modulus | $K_{u} = K + \alpha_{B}^{2}M$ |  $15.14\,\mathrm{GPa}$ |
| Shear modulus | $G = \frac{3K(1 - 2\nu)}{2(1 + \nu)}$ | $6\,$GPa |
| Undrained Poisson's ratio | $\nu_{u} = \frac{3 K_{u} - 2G}{2(3 K_{u} + G)}$ | $0.325$ |


## Earth tides

The gravitational acceleration at any point on the earth is influenced by the position of the moon, sun and the planets.  Its magnitude changes by approximately $10^{-6}\,$m.s$^{-2}$ (one part in $10^{7}$).  The impact of this small change on fluids in porous rocks has been used to understand many aspects of reservoirs, for instance: for quantifying water near a gas well [!citep](langas2006) and monitoring migration of CO$_{2}$ [!citep](sato06) and [!citep](sato_horne_18).

### Earth tides: strain

Changes in the gravitational acceleration cause the earth to change shape very slightly.  This translates locally into a non-zero strain tensor.  The strain tensor is time dependent and different for all points on the surface of the earth.  The frequency spectrum of the strain tensor is extremely complicated because of the problem's complexity (periodicity of the earth's spin, its rotation around the sun, the moon's periodicity, the earth's ellipticity, etc).  There are sophisticated software packages, for instance [!citet](rau2018), that predict the strain tensor given a time and place on the earth.  Their calculations are based on catalogues of sinusoidal functions.  [dbps_strain_fig] (Figure 2.3 in Doan, Brodsky, Prioul and Signer) shows the output of one software package.  These authors state that volumetric strain magnitudes are around $5\times 10^{-8}$.

!media media/porous_flow/dbps_strain_fig.png caption=Figure from Doan, Brodsky, Prioul and Signer showing strain-tensor components for a particular place and time window.  id=dbps_strain_fig

I think it likely that direct measurements of strain would be very useful when modelling real situations.  This could be accomplished using fibre optics [!citep](becker_coleman_19).

### Earth tides: a MOOSE model

Now consider a fully-saturated, confined aquifer.  PorousFlow may be used to simulate the porepressure response to these applied strains.

!listing modules/porous_flow/examples/tidal/earth_tide_fullsat.i

In this setup, the porepressure, $p$, is controlled by the undrained bulk modulus and the Skempton coefficient
\begin{equation}
\delta p = -B\sigma_{ii}^{\mathrm{total}}/3 = -BK_{u}\epsilon_{ii} \ ,
\end{equation}
where the repeated $i$ index indicates summation ($\sigma_{ii} = \sigma_{xx} + \sigma_{yy} + \sigma_{zz}$).  Substituting numbers yields
\begin{equation}
\label{eq:skempton_et_fullsat}
\delta p = 8.57 \times 10^{9}\epsilon_{ii} \ .
\end{equation}

For a time-independent strain field, PorousFlow reproduces [eq:skempton_et_fullsat] exactly.  For a synthetically-generated time-dependent strain field, the porepressure changes accordingly, as shown in [earth_tide_fullsat_fig].

!media media/porous_flow/earth_tide_fullsat.png caption=Example time-dependent strain field, and corresponding porepressure change in a confined aquifer.  id=earth_tide_fullsat_fig

### Earth tides: varying $g$

Realistic material properties and strain magnitudes have been used in the above MOOSE example, and the changes in porepressure are around $1\,$kPa.  This is much more than the change due to small variations in the gravitational acceleration, which would be around $\rho (\delta g) h$.  For liquid water of density $\rho\approx 1000\,$kg.m$^{-3}$ at the bottom of a water column of height $h\approx 1000\,$m, a change of $\delta g \approx 10^{-6}\,$m.s$^{-2}$, results in a porepressure change of around $1\,$Pa.  For a shallower case with $h\approx 100\,$m, the porepressure change is even smaller ($0.1\,$Pa).

At the time of writing (August 2019) the gravitational acceleration vector in MOOSE and PorousFlow cannot be time-dependent.  This is not a difficult addition to make (using `AuxVariables`) but the arguments in the preceding paragraph suggest it is unnecessary.

Currently, I'm wondering about the impact of heterogeneity on these arguments.  A more realistic MOOSE model would include different lithologies.  The boundary conditions on the displacement Variables could simply result from the earth strains (as in the above PorousFlow example).  Heterogeneity would then mean the strain was not uniform throughout the model.  Does this mean that changes in $g$ become more important?

## Barometric and oceanic loading

Conceptually, barometric and oceanic loading are similar as they both serve to change porepressure and load the porous matrix.  A numerical model must consider whether boreholes are open to the atmosphere, or sealed, and whether the aquifers are confined or unconfined.  Interestingly, [!citet](doanET2006) report that ocean tides can impact porous flows $100\,$km inland ($7^{\circ}$ lag in the Pinon Flat Observatory).  On the other hand, CSIRO has observed no impact of ocean tides only $10\,$km inland.

### Boundary conditions for barometric and oceanic loading

Conceptually, external fluids such as the atmosphere, ocean, borehole fluids or hydraulic-fracturing fluids mechanically load the porous matrix.  Mathematically, the external fluid pressure acts as an applied *total stress* at the boundary of the porous matrix (for instance, see [!citet](detcheng_hy91)).

Conceptually, the porepressure responds instantly to the external fluid pressure.  Mathematically, the porepressure (of the relevant phase) at the boundary of the porous matrix is equal to the external fluid pressure.


### BCs: total stress

The total stress, $\sigma^{\mathrm{total}}$, at the interface of the porous-matrix and the atmosphere must be equal to the atmospheric pressure, $p_{\mathrm{atm}}$:
\begin{equation}
\label{eqn.tot_stress_bc}
\sigma_{nn}^{\mathrm{total}} = -p_{\mathrm{atm}} \ .
\end{equation}
The same holds for the oceanic pressure.  Here $n$ labels the normal direction and the negative sign comes because $\sigma<0$ corresponds to a compressive stress, which is $p>0$.  This is boundary condition arises because the atmosphere or ocean is physically pushing on the porous matrix.

### BCs: porepressure

The porepressure, $p$, at the interface of the porous-matrix and the atmosphere must be equal to the atmospheric pressure:
\begin{equation}
\label{eqn.pp_bc}
p = p_{\mathrm{atm}} \ .
\end{equation}
In a multi-phase scenario, this equation would read $p_{\mathrm{gas}} = p_{\mathrm{atm}}$.

Combining [eqn.tot_stress_bc] and [eqn.pp_bc] in the single-phase case means the effective stress at the interface is
\begin{equation}
\sigma_{nn}^{\mathrm{eff}} = \sigma_{nn}^{\mathrm{tot}} + \alpha_{B}p = -(1 - \alpha_{B})p_{\mathrm{atm}} \ .
\end{equation}
The Biot coefficient, $\alpha_{B}$, can be interpreted as an effective way of accounting for pores that aren't filled with water.  When $\alpha_{B}=1$ all the porespace is filled with water, and the above equation reads $\sigma_{nn}^{\mathrm{eff}}=0$, so an increase in atmospheric pressure produces zero strain: the porewater pushes back on the porous skeleton with the same magnitude as the atmosphere.  When $\alpha_{B}<1$, the above equation means that an increase in atmospheric pressure results in a compression: the porewater doesn't fill tne entire porespace so isn't as effective at "pushing back" and can't match the load of the atmospheric pressure.

### BCs: within a water-filled borehole

If a borehole is open, then the bottomhole pressure varies linearly with the atmospheric pressure: $\delta p_{\mathrm{bottomhole}} = \delta p_{\mathrm{atm}}$.

For a sealed section of a well, the borehole pressure is the same as the porepressure within the adjacent rock.

## Atmospheric loading on a fully-confined, fully-saturated aquifer: barometric efficiency

Here, "fully confined" means the aquifer is completely enclosed by aquitards of zero permeability.  Imagine the fully-saturated aquifer is horizontal, has infinite extent and a uniform thickness.  It is undrained for its water cannot escape.  This example is artificially idealised: sections below explain how to extend this analysis to partially-confined or unconfined aquifers.

Because of the fully-confined nature (zero-permeability aquitards), any porepressure changes at the topography are not felt in the aquifer.  Therefore, [eqn.tot_stress_bc] applied to the aquifer's top is the only non-trivial BC.  As for the other BCs: it is assumed that aquifer's other boundaries are fixed, and all boundaries are impermeable to water.  The input file is:

!listing modules/porous_flow/examples/tidal/barometric_fully_confined.i

[!citet](doanET2006) introduce this example, and state the porepressure response in their Equation (3.1):
\begin{equation}
p = \gamma p_{\mathrm{atm}} = \frac{B(1 + \nu_{u})}{3(1-\nu_u)}p_{\mathrm{atm}} \ .
\end{equation}
Here $\gamma$ is called the *barometric efficiency*.  For the parameters listed in [earth_tide_fullsat_params] it is $\gamma = 0.3704$.  The MOOSE model reproduces this exactly.



## Atmospheric loading on an unconfined, fully-saturated aquifer

Changes of atmospheric pressure directly impact the porepressure within the rock, as well as acting on the rock skeleton.  Consider the model shown in [atm_tides_fig].

!media media/porous_flow/atm_tides.png style=width:20% caption=Geometry of the unconfined, fully-saturated aquifer.  id=atm_tides_fig


Use the following assumptions.

- The atmospheric pressure varies as $p_{\mathrm{atm}} = 5 \mathrm{sin}(2\pi t)\,$kPa.

- The boundaries are impermeable except the top boundary

- Only vertical displacement is allowed

- The barometric loading acts on the total stress at the model's top, as in [eqn.tot_stress_bc]

The MOOSE input file is:

!listing modules/porous_flow/examples/tidal/atm_tides.i

The results in [atm_tides_p_uz] show that, in this model, a realistic barometric fluctuation of $5\,$kPa produces a vertical displacement of about $0.02\,$mm and that the porepressure at depth experiences a phase lag and amplitude reduction, as expected.  [atm_tides_p_uz] shows the first few sinusoidal periods.  After about 5 periods, transient responses have more-or-less disappeared.

!media media/porous_flow/atm_tides_p_uz.png caption=Response of the fully-saturated, unconfined aquifer to cyclic barometric pressure.  The first few sinusoidal periods are shown: after this time transient behaviour has mostly disappeared.  id=atm_tides_p_uz


## Atmospheric loading on an unconfined, fully-saturated aquifer containing an open borehole

This model includes a borehole.  The borehole is assumed to be cased, except for the very bottom section at 100m depth.  Therefore, groundwater seeps into the borehole and rises up the hole until it reaches the ground level (where porepressure is zero).  Furthermore, it is assumed in this example that the borehole is always full of water (for example, as water is pushed into the rock skeleton it is magically replenished).  Similarly it is assumed the rock skeleton is always fully saturated.  The setup is shown in [atm_tides_open_hole_fig] and the results are shown in [atm_tides_open_hole_p_uz].  [atm_tides_open_hole_p_uz] shows the first few sinusoidal periods.  After about 5 periods, transient responses have more-or-less disappeared.


!media media/porous_flow/atm_tides_open_hole.png style=width:60% caption=Geometry of the unconfined, fully-saturated aquifer containing a borehole that is cased to 100m.  The porepressure observation point is shown in red.  id=atm_tides_open_hole_fig

!listing modules/porous_flow/examples/tidal/atm_tides_open_hole.i

!media media/porous_flow/atm_tides_open_hole_p_uz.png caption=Response of the fully-saturated, unconfined aquifer containing an open borehole to cyclic barometric pressure.  The first few sinusoidal periods are shown: after this time transient behaviour has mostly disappeared.  id=atm_tides_open_hole_p_uz




## Bibliography
