# Binary Gas Mixture Flow Model

This model builds on the
[compressible flow model](modules/thermal_hydraulics/theory_manual/vace_model/index.md)
using a binary (two-component) gas mixture. In this section we do not repeat the
aforementioned theory but just highlight the differences introduced by the additional gas
component.
The mass, momentum, and energy equations remain largely unchanged, except that
in most cases, the properties correspond to the mixture properties.
Note the following notation: the subscript $v$ corresponds to the primary gas component,
the subscript $g$ corresponds to the secondary gas component, and no subscript
is used for mixture quantities.
An additional equation is added to the system for the continuity equation
for the species $g$, as derived in [!citep](hansel2018ncgs):

!equation id=diff_species
\pd{\xi_g \rho A}{t} + \pd{\xi_g \rho u A}{x} = -\pd{J_g}{x} \,,

!equation
J_g = - \rho D \pd{\xi_g}{x} A \,,

where $\xi_k$ is the mass fraction for component $k$, $J_k$ is the diffusive flux of component $k$,
approximated using Fick's first law, and $D$ is the binary diffusion coefficient.

Additionally, an energy term is added to the energy equation:

!equation id=diff_energy
\pd{\rho E A}{t} + \cdots = -\sum\limits_k \pd{J_k H_k}{x} \,.

## Spatial Discretization

Here we describe how the energy diffusion term in [!eqref](diff_energy) is computed in our spatial discretization. The mass diffusion term in [!eqref](diff_species) is computed similarly.

After integrating over an element $i$ for the finite volume discretization,

!equation
\sum\limits_k \left(J_{k,i+1/2} H_{k,i+1/2} - J_{k,i-1/2} H_{k,i-1/2}\right)

Considering a single edge $i+1/2$,

!equation
J_{k,i+1/2} = -\rho_{i+1/2} D_{i+1/2} \left.\pd{\xi_k}{x}\right|_{i+1/2} \eqc

where the density and the diffusion coefficient are computed using linear interpolation between the $i$ and $i+1$ cell-center values:

!equation
y_{i+1/2} = y_i + (x_{i+1/2} - x_i) \frac{y_{i+1} - y_i}{x_{i+1} - x_i} \eqc

and the mass fraction gradient is computed using the slope between the adjacent cell-center values:

!equation
\left.\pd{\xi_k}{x}\right|_{i+1/2} = \frac{\xi_{k,i+1} - \xi_{k,i}}{x_{i+1} - x_i} \eqp

The specific total enthalpy is computed as follows, where edge quantities are computed as linear interpolations as shown above:

!equation
\psi_{k,i+1/2} = \psi(\xi_{k,i+1/2}) \eqc

!equation
p_{k,i+1/2} = \psi_{k,i+1/2} p_{i+1/2} \eqc

!equation
\hat{u}_{k,i+1/2} = \frac{J_{k,i+1/2}}{\xi_{k,i+1/2} \rho_{i+1/2}} \eqc

!equation
u_{k,i+1/2} = u_{i+1/2} + \hat{u}_{k,i+1/2} \eqc

!equation
H_{k,i+1/2} = e_k(p_{k,i+1/2}, T_{i+1/2}) + \frac{p_{k,i+1/2}}{\rho_k(p_{k,i+1/2}, T_{i+1/2})} + \frac{1}{2} u_{k,i+1/2}^2 \eqp

## Binary Diffusion Coefficient

The binary diffusion coefficient depends upon the following:

- mixture composition
- temperature, $T$
- pressure, $p$

For a binary mixture of ideal gases, kinetic theory shows the following [!citep](incropera2002)
relationship for temperature and pressure:

!equation
D \propto \frac{T^{3/2}}{p} \,.

This is useful for estimating the diffusion coefficient when the value is known
at some other $T$ and $p$ conditions.
For reference, at one atmosphere, the binary diffusion coefficient between
water and air at 298 K is $0.26\times 10^{-4}$ m$^2$/s [!citep](incropera2002),
which is a typical order of magnitude for this coefficient.
The diffusion coefficient can be computed from first principles, which
generally is within an error of 10%, but in general this is not practical for
non-specialists. Instead it is recommended to search literature for appropriate
values, which may or may not be available, depending on the how common the
gas pairing is.

## Initial Conditions

Initial conditions must now specify the initial, secondary gas mass fraction $\xi_{g,0}$,
in addition to the initial pressure $p_0$, initial temperature $T_0$, and
initial velocity $u_0$:

!equation
(\xi_g \rho A)_0 = \xi_{g,0} \rho_0 A \,,

!equation
(\rho A)_0 = \rho_0 A \,,

!equation
(\rho u A)_0 = \rho_0 u_0 A \,,

!equation
(\rho E A)_0 = \rho_0 E_0 A \,,

where the initial density $\rho_0$ and initial specific total energy $E_0$
are computed as follows:

!equation
\rho_0 = \rho(p_0, T_0, \xi_{g,0}) \,,

!equation
e_0 = e(p_0, T_0, \xi_{g,0}) \,,

!equation
E_0 = e_0 + \frac{1}{2} u_0^2 \,.
