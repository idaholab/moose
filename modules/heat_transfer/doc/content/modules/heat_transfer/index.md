# Heat Transfer Module

Heat is transferred by three mechanisms: conduction, convection, and radiation.
The heat transfer module provides various implementations of the heat conduction
equation, as well as associated boundary/interface conditions, including radiation
between opaque, gray, diffuse surfaces and provisions to couple temperature fields to
fluid domains through boundary conditions.

## Tutorial Problems

A [tutorial](modules/heat_transfer/tutorials/introduction/index.md) for the Heat Transfer module is available to show the steps for setting up a basic model.

## Basic Theory of Heat Transfer

The heat conduction equation describes the diffusion of heat in a solid or stationary fluid:

\begin{equation}\label{eq:heat_equation}
  \rho(t, \vec{x}) c(t, \vec{x})\frac{\partial T}{\partial t} = \nabla k(t,\vec{x}) \nabla T + \dot{q} ~\text{for}~\vec{x} \in \Omega,
\end{equation}

where $T$ is temperature, $t$ is time, $\vec{x}$ is the vector of spatial coordinates, $\rho$ is the density, $c$ is the specific heat capacity, $k$ is the thermal conductivity, $\dot{q}$ is a heat source density, and $\Omega$ is the domain. It should be emphasized that for solids the isobaric and isochoric heat capacities, $c_p$ and $c_v$ respectively, are almost identical, while for stagnant fluids $c_p$ should be used.

Boundary conditions for the heat equation are defined on the boundary of the domain $\partial \Omega$. The boundary is divided into Dirichlet boundaries $\partial \Omega_D$ and Robin boundaries $\partial \Omega_R$ such that $\partial \Omega = \partial \Omega_D \cup \partial \Omega_R$:

\begin{equation}
\begin{array}{l l}
   T(t,\vec{x}) = T_D(t,\vec{x}) & \vec{x}\in \partial \Omega_D  \\
   -k \vec{n} \cdot \nabla T - G(t,\vec{x},T) = 0 & \vec{x}\in \partial \Omega_R,
\end{array}
\end{equation}

where $T_D$ and $G(t,\vec{x},T)$ are known functions and $\vec{n}$ is the outward normal at $\vec{x}$. The function $G(t,\vec{x},T)$ defines the type of Robin
boundary conditions. Common cases for $G$ are:

\begin{equation}
\begin{aligned}\label{eq:robin_bc_types}
   &\text{Neumann:     }~G(t,\vec{x},T) = 0 \\
   &\text{Convection:  }~G(t,\vec{x},T) = h (T - T_{\infty}) \\
   &\text{Radiation:   }~G(t,\vec{x},T) = \sum\limits_{j=1}^I F_{i,j} (J_i - J_j) ~\text{ on surface }i,
\end{aligned}
\end{equation}

where $j$ loops over all surfaces participating in the radiative exchange, $F_{i,j}$ is the view factor from the $i$-th to the $j$-th surface, and $J_j$ is the radiosity of the $j$-th defined by:

\begin{equation}
 J(t, \vec{x}) =  \epsilon(t,\vec{x}) \sigma T^4   + (1 - \epsilon(t,\vec{x})) H(t, \vec{x}),
\end{equation}

where $\epsilon(t,\vec{x})$ is the emissivity, $\sigma = 5.67 \times 10^{-8} \frac{W}{\text{m}^2 \text{K}^4}$ is the Stefan-Boltzmann constant, and $H(t,\vec{x})$ is the total irradiation. Often the surface of interest is in radiative transfer with just a single isothermal surface that completely encloses it. In this case $G(t,\vec{x},T)$ is given by:

\begin{equation}
 G(t,\vec{x},T) = \sigma S \left(T^4 - T^4_{\infty}\right),
\end{equation}

where $S$ depends on the geometry, e.g.,
\begin{equation}
  \begin{aligned}
     &\text{Infinite cylinder }~S = \frac{\epsilon ~\epsilon_C R_C}{\epsilon_C R_C + \epsilon R (1 - \epsilon_C)},
  \end{aligned}
\end{equation}

where $\epsilon_C$ is the emissivity of the enclosing cylinder, $R_C$ is the radius of the enclosing cylinder, and $R$ is the outer radius of the domain that is assumed to be cylindrical as well.

## Theory of Opaque, Gray, Diffuse Radiative Exchange id=gray_diffuse_radiative_exchange

This section discusses the pertinent equations of the net radiation method and relates them to the
more fundamental spectral intensity.
The development of the method is taken from [!cite](modest2013radiative).
The net radiation method computes the radiative exchange
between surfaces for the case of no attenuation in the medium between the surfaces. For a point on
any of the surfaces in radiative heat transfer, a heat balance is formulated at location $\vec{x}$:

\begin{equation}\label{eq:heat_balance}
  q(\vec{x}) = J(\vec{x}) - H(\vec{x}),
\end{equation}

where $J$ is the radiosity of the surface at point $\vec{x}$, and $H$ is the irradiation from other
surface to point $\vec{x}$. The units of all quantities in [eq:heat_balance] are $\text{W}/{\text{m}^2}$.
The radiosity is the sum of the emissive power and the reflected portion of the
irradiation:

\begin{equation}\label{eq:radiosity}
  J(\vec{x}) = \epsilon(\vec{x}) \sigma T^4 + \rho(\vec{x}) H(\vec{x}),
\end{equation}

where $\epsilon$ is the emissivity, and $\rho$ is the reflectivity.
For opaque, gray, diffuse surfaces, the reflectivity simply is $\rho = 1 - \epsilon$:

\begin{equation}\label{eq:radiosity_gray}
  J(\vec{x}) = \epsilon(\vec{x}) \sigma T^4 + (1 - \epsilon(\vec{x})) H(\vec{x}).
\end{equation}

At this point it is convenient to represent the radiosity, temperature, irradiation,
and emissivity on each distinct surface $i$ by a suitable average of the actual distribution.
We leave out the $\vec{x}$ argument and add an $i$ index by which we indicate that this
quantity has been averaged over the extent of surface $i$.

A second balance equation in addition to [eq:radiosity_gray] is obtained by relating
the irradiation onto surface $i$ to the radiosities of all other surfaces:

\begin{equation}\label{eq:second_balance}
  H_i = \sum\limits_{j=1}^n F_{i,j} J_j,
\end{equation}
where $n$ is the total number of surfaces, and $F_{i,j}$ is the view
factor from surface $i$ to surface $j$. Eliminating $H_i$ from [eq:second_balance] using
[eq:heat_balance] gives:

\begin{equation}\label{eq:fixed_q}
  \sum\limits_{j=1}^n \left( \delta_{i,j} - F_{i,j }\right) J_j = q_i.
\end{equation}

This result is used for computing $J_j$ on surfaces where $q_i$ is known. This is the case for
`ADIABATIC` surfaces, where $q_i = 0$.

A more convenient relationship is derived for surfaces where $T_i$ is either known (`FIXED_TEMPERATURE`) or
computed (`VARIABLE_TEMPERATURE`). We first assert that because [eq:heat_balance] and [eq:radiosity_gray] hold pointwise, they also hold for the averages over face $i$. This is denoted by switching from $J(\vec{x})$ to $J_i$ and similarly from $H(\vec{x})$ to $H_i$. Then, solving [eq:heat_balance] for $H_i$, eliminating $H_i$ in [eq:radiosity_gray], and solving for $q$ gives:

\begin{equation}
 q_i = \frac{\epsilon_i}{1 - \epsilon_i} \left( \sigma T_i^4 - J_i \right).
\end{equation}
Then we use this equation in [eq:fixed_q] to eliminate $q_i$:
\begin{equation}\label{eq:fixed_T}
 \sum\limits_{j=1}^n \left( \delta_{i,j} - (1 - \epsilon_i) F_{i,j }\right) J_j = \epsilon_i \sigma T_i^4.
\end{equation}

The net radiation method implements [eq:fixed_q] with $q=0$ on `ADIABATIC` boundaries, and [eq:fixed_T] on `FIXED_TEMPERATURE` and `VARIABLE_TEMPERATURE` boundaries. `FIXED_TEMPERATURE` boundaries allow the temperature to vary as a user-defined function, while `VARIABLE_TEMPERATURE` walls use a MOOSE variable to represent temperature. In both cases, $T$ is not constant over the extent of the surface. We average the right hand side of [eq:fixed_T] as follows:

\begin{equation}
  \sum\limits_{j=1}^n \left( \delta_{i,j} - (1 - \epsilon_i) F_{i,j }\right) J_j = \beta_i,
\end{equation}

where

\begin{equation}
  \beta_i =  \frac{ \epsilon_i  \sigma}{A_i} \int_{A_i} T_i^4 dA,
\end{equation}

where $A_i$ is the area of surface $i$.
The `GrayLambertSurfaceRadiationBase` object computes the average temperature $T_i$, heat flux $q_i$, and radiosity $J_i$.
These are all average quantities over the surface $i$.

## Relationship of Net Radiation Method with Radiative Transport

The radiative transport equation is formulated in terms of the spectral intensity $I(\vec{x}, t, \lambda, \hat{\Omega})$ with $\lambda$ being the photon's wavelength and $\hat{\Omega}$ being the direction
of propagation. The physical meaning of the spectral intensity is that the energy transported through
area $dA$ at $\vec{x}$, during time interval $dt$ at time $t$, along direction within a cone $d\hat{\Omega}$ about $\hat{\Omega}$ and with wavelengths within $d \lambda$ about $\lambda$ is:

\begin{equation}
  \text{Energy} = I(\vec{x}, t, \lambda, \hat{\Omega}) dt dA |\vec{n} \cdot \hat{\Omega}| d\hat{\Omega} d\lambda,
\end{equation}

where $\vec{n}$ is the normal on face $dA$.
The intensity is computed by a code that solves the radiative transfer equation, e.g., [Griffin](https://inlsoftware.inl.gov/product/griffin) within the MOOSE framework. The solution of the radiative transfer equation is coupled to heat conduction within solid domains
through the balance at the surface [eq:heat_balance], but now we evaluate the net heat flux from the spectral intensity:

\begin{equation}
  q(\vec{x}) = \int\limits_{4 \pi} d\hat{\Omega} \int\limits_{0}^{\infty} d\lambda (\vec{n} \cdot \hat{\Omega}) I(\vec{x}, t, \lambda, \hat{\Omega}).
\end{equation}

The boundary condition for the thermal radiation equation on a gray, diffuse surface is:
\begin{equation}\label{eq:rad_transfer_bc}
  I(\vec{x}, t, \lambda, \hat{\Omega}) = \frac{1}{\pi}\left[\frac{\epsilon(t,\vec{x},\lambda) n^2 2 \pi h c_0^2}{\lambda^5 \left[ \exp{\frac{h c_0}{k_B \lambda T}} - 1 \right]} + \left(1 - \epsilon(t,\vec{x},\lambda)\right) \int\limits_{\hat{\Omega}' \cdot \vec{n} > 0}  \hat{\Omega}' \cdot \vec{n} ~I(\vec{x}, t, \lambda, \hat{\Omega}') d \hat{\Omega}' \right]~\text{for}~\vec{n} \cdot \hat{\Omega} < 0,
\end{equation}

where $n$ is the refractive index of the medium that the surface radiates into, $\epsilon(t,\vec{x},\lambda)$ is the emissivity of the surface at wavelength $\lambda$, $h$ is Planck's constant, $k_B$ is Boltzmann's constant, and $c_0$ is the speed of light in vacuum.
First, it should be stressed that the refractive index is very close to unity in vacuum or a weakly absorbing gas, but may differ significantly from unity in semi-transparent media. As the net radiation method does not apply to semi-transparent materials, $n$ is assumed to be unity. Second, the normalization factor of $1/\pi$ in [eq:rad_transfer_bc] is for three-dimensional geometry; more accurately it is given by $\frac{1}{4} \int d \hat{\Omega}$, i.e. size of the unit sphere divided by four. Third, radiation effects in semi-transparent media can be modeled with the MOOSE based radiation transport code Rattlesnake.

## Including Adiabatic and Isothermal Boundaries

Boundaries can be declared adiabatic or isothermal boundaries. Adiabatic and isothermal boundaries do not participate in the heat conduction solution process, i.e. no temperature variable needs to be defined on them (if one is defined it is not used). Adiabatic and isothermal boundaries are convenient if the outer boundary of the heat conduction domain is in radiative transfer with a wall that is either insulated or kept at a constant temperature. For example, think of the outside of a reactor pressure vessel that is in radiative heat transfer with the reactor cavity cooling system. The reactor cavity cooling system may be modeled as isothermal wall, while the top and bottom of the cavity may be modeled as adiabatic walls.
The temperature equation is not solved on of these surfaces, but they are in radiative transfer with another surface on which temperature is defined as a variable.

## Implementation of Opaque, Gray, Diffuse Radiative Exchange

The implementation of the net radiation method in MOOSE relies on the following types of objects:

- Objects deriving from `GrayLambertSurfaceRadiationBase` are `SideUserObjects` that compute radiosities,
  heat fluxes, and average temperatures for a set of sidesets involved in radiative heat exchange.
  The net radiation method is implemented in `GrayLambertSurfaceRadiationBase`, which provides a public
  interface to the average heat flux, radiosities, and average temperatures computed for each sideset.
  These objects differ in how they are provided with view factors:

  - [ConstantViewFactorSurfaceRadiation.md], for providing view factors manually
  - [ViewFactorObjectSurfaceRadiation.md], for providing view factors via an object deriving from `ViewFactorBase`

  Note that a temperature variable must be available on sidesets that are not marked as isothermal or adiabatic.

- Objects inheriting from `ViewFactorBase`, such as the following:

  - [UnobstructedPlanarViewFactor.md]
  - [RayTracingViewFactor.md]

  compute view factors and provide them via the `getViewFactor` public interface. These objects are used by [ViewFactorObjectSurfaceRadiation.md], which inherits from  `GrayLambertSurfaceRadiationBase`.

- [GrayLambertNeumannBC.md] takes the average heat flux computed in `GrayLambertSurfaceRadiationBase`
  and applies it as boundary condition as described in [eq:robin_bc_types].

- [GrayLambertSurfaceRadiationPP.md] is a post-processor that retrieves the radiosity, heat flux, or temperature
  for a boundary used in a `GrayLambertSurfaceRadiationBase` object.

- [ViewFactorPP.md] is a post-processor that can retrieve a view factor from a `ViewFactorBase` for output purposes.

- [SurfaceRadiationVectorPostprocessor.md] is a vector post-processor that retrieves one or more of the following
  for all surfaces for a given `GrayLambertSurfaceRadiationBase` object: emissivity, radiosity, temperature, and heat flux.

- [ViewfactorVectorPostprocessor.md] is a vector post-processor that retrieves the view factors
  between all boundaries for a given `GrayLambertSurfaceRadiationBase` object.

- Future plans include the addition of an action to set up view-factor net radiation transfer using only a single syntax block.

## Gap Heat Transfer

There are multiple approaches in the Heat Transfer module for modeling heat
transfer across a gap:

- [ModularGapConductanceConstraint.md] uses the mortar finite element method and
  may be used when the gap is based on the distance between mesh faces.
- [SideSetHeatTransferKernel.md] is an [InterfaceKernel](InterfaceKernels/index.md)
  that models heat transfer across a side set, via conduction, convection, and
  radiation.
- [ThermalContact](syntax/ThermalContact/index.md) uses a "node-on-face" approach
  to model gap heat transfer and may be used when the gap is based on
  the distance between mesh faces.
- [CylindricalGapHeatFluxFunctorMaterial.md] is a [FunctorMaterial](FunctorMaterials/index.md)
  that computes heat fluxes across a cylindrical gap, where the gap thickness
  is provided via radii functors rather than taken from mesh information.

## Objects, Actions, and Syntax

!syntax complete groups=HeatTransferApp level=3
