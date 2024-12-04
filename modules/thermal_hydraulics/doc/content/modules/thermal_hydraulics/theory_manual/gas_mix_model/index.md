# Binary Gas Mixture Flow Model

This model builds on the
[compressible flow model](modules/thermal_hydraulics/theory_manual/vace_model/index.md)
using a binary (two-component) gas mixture. In this section we do not repeat the
aforementioned theory but just highlight the differences introduced by the additional gas
component.
The mass, momentum, and energy equations remain largely unchanged, except that
in most cases, the properties correspond to the mixture properties.
The gas mixture properties are computed using [/IdealRealGasMixtureFluidProperties.md],
which uses Dalton's law of partial pressures.
Note the following notation: the subscript $v$ corresponds to the primary gas component,
the subscript $g$ corresponds to the secondary gas component, and no subscript
is used for mixture quantities.
An additional equation is added to the system for the continuity equation
for the species $g$, as derived in [!citep](hansel2018ncgs):

!equation
\pd{\xi_g \rho A}{t} + \pd{\xi_g \rho u A}{x} = -\pd{J_g}{x} \,,

!equation
J_g = - \rho D \pd{\xi_g}{x} A \,,

where $\xi_g$ is the mass fraction for component $g$, $J_g$ is the diffusive flux of component $g$,
approximated using Fick's first law, and $D$ is the binary diffusion coefficient.

Additionally, an energy term is added to the energy equation:

!equation
\pd{\rho E A}{t} + \cdots = -\sum\limits_g \pd{J_g H_g}{x} \,.

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
