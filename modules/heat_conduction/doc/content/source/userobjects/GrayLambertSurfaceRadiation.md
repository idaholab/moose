# GrayLambertSurfaceRadiation

## Description

`GrayLambertSurfaceRadiation` implements the exchange of heat by radiation between
sidesets by the net radiation method.
The net radiation method is valid if the surfaces are gray, diffuse radiators.
The purpose of the `GrayLambertSurfaceRadiation` is to provide the means
to model radiative exchange for coarse-grained model. `GrayLambertSurfaceRadiation`
does not compute radiative exchange between element surfaces, but it computes radiative
transfer only in an average sense between sidesets.

The `GrayLambertSurfaceRadiation` allows coupling radiative heat transfer to regions
where the heat equation is solved. The net heat transfer caused by radiative heat transfer is
coupled to the temperature field by `GrayLambertNeumannBC`. `GrayLambertSurfaceRadiation` also supports the
definition of adiabatic and fixed temperature sidesets. The temperature variable, i.e. the
variable of the heat conduction equation, does not need to be defined on adiabatic and fixed
temperature boundaries. This is particularly useful in cavities, where temperature is only
defined on sidesets immediately adjacent to the heat conduction domain and not on the
adiabatic and isothermal walls enclosing the cavity. There are three different types of boundary
conditions in `GrayLambertSurfaceRadiation`:

- `VARIABLE_TEMPERATURE` are sidesets where temperature is provided by the `temperature` variable.
  The heat equation is coupled to the `GrayLambertSurfaceRadiation` through these boundaries.

- `FIXED_TEMPERATURE` are sidesets with temperature given as a function. The difference to `VARIABLE_TEMPERATURE`
  is that we do not need to solve for temperature on `FIXED_TEMPERATURE`. `FIXED_TEMPERATURE` sidesets
  can for example be the outside of a cavity that is in radiative heat transfer with the domain
  but is kept at constant temperature by being in contact with an effective coolant.

- `ADIABATIC` sidesets have a zero net heat-flux. This implies that the inflow of radiation has
  to equal the outflow of radiation. The surface temperature assumes the value at which this condition
  is true. `ADIABATIC` sidesets do not require the temperature variable to be defined.

## Explanation of the input parameters

This paragraph describes the input structure of the `GrayLambertSurfaceRadiation` object.
The following parameters are defined in detail:

- `temperature`: the user must provide the name of the temperature variable as the `temperature` parameter.
  `temperature` is the variable that the heat conduction equation solves for.

- `boundary` contains the names of _all_ sidesets that participate in the radiative heat exchange.

- `fixed_temperature_boundary` should list all sidesets that are fixed temperature boundaries.
  `fixed_temperature_boundary` must be a subset of the `boundary` array.

- `fixed_boundary_temperatures` contains function names specifying the temperatures on the fixed
  temperature boundaries. `fixed_boundary_temperatures` must be the same length as `fixed_temperature_boundary`.

- `adiabatic_boundary` should list all sidesets that are adiabatic boundaries.
  `adiabatic_boundary` must be a subset of the `boundary` array.

- `view_factors` lists the view factors $F_{i,j}$ from sideset $i$ to sideset $j$. The ordering of the
  `view_factors` follows the ordering in `boundary`. View factors must be added as square arrays. This may
  appear redundant because missing view factors can be computed using reciprocity, but this makes it impossible
  to check that each row of the view factor matrix sums to 1. This is absolutely necessary for conserving
  energy and ensuring stability of a coupled radiative transfer, heat conduction calculation. Row sums that
  are less than 5 percent off, are corrected to sum to 1. If the row sum is off by more than this threshold,
  an error is thrown. This is an example of the format of `view_factors` for three surfaces:

  \begin{equation}
      '~~F_{1,1} F_{1,2} F_{1,3} ;
       F_{2,1} F_{2,2} F_{2,3} ;
       F_{3,1} F_{3,2} F_{3,3} ~~'
  \end{equation}

## The Equations of Gray Lambert Radiators

This section discusses the pertinent equations of the net radiation method and relates them to the
more fundamental spectral intensity.
The development of the method is taken from [!cite](modest2013radiative).
The net radiation method computes the radiative exchange
between surfaces for the case of no attenuation in the medium between the surfaces. For a point on
any of the surfaces in radiative heat transfer, a heat balance is formulated at location $\vec{r}$:

\begin{equation}\label{eq:heat_balance}
  \dot{q}(\vec{r}) = J(\vec{r}) - H(\vec{r}),
\end{equation}

where $J$ is the radiosity of the surface at point $\vec{r}$ and $H$ is the irradiation from other
surface to point $\vec{r}$. The units of all quantities in [eq:heat_balance] is $\text{W}/{\text{m}^2}$.
The radiosity is the sum of the emissive power and the reflected portion of the
irradiation:

\begin{equation}\label{eq:radiosity}
  J(\vec{r}) = \epsilon(\vec{r}) \sigma T^4 + \rho(\vec{r}) H(\vec{r}),
\end{equation}

where $\epsilon$ is the emissivity and $\rho$ is the reflectivity.
For gray, diffuse surfaces, the reflectivity simply is $\rho = 1 - \epsilon$:

\begin{equation}\label{eq:radiosity_gray}
  J(\vec{r}) = \epsilon(\vec{r}) \sigma T^4 + (1 - \epsilon(\vec{r})) H(\vec{r}).
\end{equation}

At this point it is convenient to represent the radiosity, temperature, irradiation,
and emissivity on each distinct surface $i$ by a suitable average of the actual distribution.
We leave out the $\vec{r}$ argument and add an $i$ index by which we indicate that this
quantity has been averaged over the extent of surface $i$.

A second balance equation in addition to [eq:radiosity_gray] is obtained by relating
the irradiation onto surface $i$ to the radiosities of all other surfaces:

\begin{equation}\label{eq:second_balance}
  H_i = \sum\limits_{j=1}^n F_{i,j} J_j,
\end{equation}
where $A_i$ is the area of surface $i$, $n$ is the total number of surfaces, and $F_{i,j}$ is the view
factor from surface $i$ to surface $j$. Eliminating $H_i$ from [eq:second_balance] using
[eq:radiosity_gray] gives:

\begin{equation}\label{eq:fixed_q}
  \sum\limits_{j=1}^n \left( \delta_{i,j} - F_{i,j }\right) J_j = \dot{q}_i.
\end{equation}

This result is used for computing $J_j$ on surfaces where $\dot{q}_i$ is known. This is the case for
`ADIABATIC` surfaces, where $\dot{q}_i = 0$.

A more convenient relationship is derived for surfaces where $T_i$ is either known (`FIXED_TEMPERATURE`) or
computed (`VARIABLE_TEMPERATURE`). From [eq:radiosity_gray] and [eq:heat_balance], we obtain:

\begin{equation}
 \dot{q}_i = \frac{\epsilon_i}{1 - \epsilon_i} \left( \sigma T_i^4 - J_i \right).
\end{equation}
Then we use this equation in [eq:fixed_q] to eliminate $\dot{q}_i$:
\begin{equation}\label{eq:fixed_T}
 \sum\limits_{j=1}^n \left( \delta_{i,j} - (1 - \epsilon_i) F_{i,j }\right) J_j = \epsilon_i \sigma T_i^4.
\end{equation}

## Relationship of Net Radiation Method with Radiative Transport

The radiative transport equation is formulated in terms of the spectral intensity $I(\vec{r}, t, \lambda, \hat{\Omega})$ with $\lambda$ being the photon's wavelength and $\hat{\Omega}$ being the direction
of propagation. The physical meaning of the spectral intensity is that the energy transported through
area $dA$ at $\vec{r}$, during time interval $dt$ at time $t$, along direction within a cone $d\hat{\Omega}$ about $\hat{\Omega}$ and with wavelengths within $d \lambda$ about $\lambda$ is:

\begin{equation}
  \text{Energy} = I(\vec{r}, t, \lambda, \hat{\Omega}) dt dA |\vec{n} \cdot \Omega| d\Omega d\lambda,
\end{equation}

where $\vec{n}$ is the normal on face $dA$.
The intensity is computed by a code that solves the radiative transfer equation, e.g. Rattlesnake within the MOOSE framework. The solution of the radiative transfer equation is coupled to heat conduction within solid domains
through the balance at the surface [eq:heat_balance], but now we evaluate the net heat flux from the spectral intensity:

\begin{equation}
  \dot{q}(\vec{r}) = \int\limits_{4 \pi} d\hat{\Omega} \int\limits_{0}^{\infty} d\lambda (\vec{n} \cdot \hat{\Omega}) I(\vec{r}, t, \lambda, \hat{\Omega}).
\end{equation}

## Implementation of the Net Radiation Method

The net radiation method implements [eq:fixed_q] with $\dot{q}=0$ on `ADIABATIC` boundaries, and [eq:fixed_T] on `FIXED_TEMPERATURE` and `VARIABLE_TEMPERATURE` boundaries. `FIXED_TEMPERATURE` boundaries allow the temperature to vary as a user-defined function, while `VARIABLE_TEMPERATURE` walls use a MOOSE variable to represent temperature. In both cases, $T$ is not constant over the extent of the surface. We average the right hand side of [eq:fixed_T] as follows:

\begin{equation}
  \sum\limits_{j=1}^n \left( \delta_{i,j} - (1 - \epsilon_i) F_{i,j }\right) J_j = \beta_i,
\end{equation}

where

\begin{equation}
  \beta_i =  \frac{ \epsilon_i  \sigma}{A_i} \int_{A_i} T_i^4 dA.
\end{equation}

This userobject computes the average temperature $T_i$, heat flux density $\dot{q}_i$, and radiosity $J_i$.
These are all average quantities over the surface $i$.

## Example Input syntax

!listing modules/heat_conduction/test/tests/gray_lambert_radiator/gray_lambert_cavity.i
block=UserObjects

!syntax parameters /UserObjects/GrayLambertSurfaceRadiation

!syntax inputs /UserObjects/GrayLambertSurfaceRadiation

!syntax children /UserObjects/GrayLambertSurfaceRadiation
