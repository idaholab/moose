# FluidPropertiesInterrogator

!syntax description /UserObjects/FluidPropertiesInterrogator

## Introduction

The `FluidPropertiesInterrogator` user object is used to query fluid properties
objects that derive from the following types:

- `SinglePhaseFluidProperties`
- `VaporMixtureFluidProperties`
- `TwoPhaseFluidProperties` (note that `TwoPhaseNCGFluidProperties` derives from this)

The user specifies a thermodynamic state at which to evaluate a number of fluid
properties. This can be useful for a number of different tasks, such as the
following:

- Determining values for initial conditions or problem setup
- Verifying out-of-bounds inputs to fluid properties interfaces
- Getting values to be used in tests

## Usage

The interrogator is used with a syntax for
[AddFluidPropertiesInterrogatorAction](/AddFluidPropertiesInterrogatorAction.md).
In an input file, the user will only need a block for the `FluidPropertiesInterrogator`
and a block for creating the fluid properties object that will be interrogated.
For convenience, an input file to use the interrogator is provided in the module:

!listing fluid_properties/fp_interrogator/fp_interrogator.i

## Valid Input Combinations

Notation is summarized in the following table:

| Symbol | Description |
| - | - |
| $p$        | Pressure |
| $p_{sat}$  | Saturation pressure |
| $p_{crit}$ | Critical pressure |
| $T$        | Temperature |
| $T_{sat}$  | Saturation temperature |
| $\rho$     | Density |
| $v$        | Specific volume |
| $e$        | Specific internal energy |
| $E$        | Specific total energy |
| $h$        | Specific enthalpy |
| $\Delta h_{\ell\rightarrow v}$ | Latent heat of vaporization |
| $s$        | Specific entropy |
| $c$        | Sound speed |
| $u$        | Fluid speed |
| $\mu$      | Dynamic viscosity |
| $c_p$      | Specific heat at constant pressure |
| $c_v$      | Specific heat at constant volume |
| $k$        | Thermal conductivity |
| $\beta$    | Volumetric expansion coefficient |
| $x_{NCG}$  | Mass fraction of non-condensable gas |

Let the set of single-phase fluid properties be defined as
\begin{equation}
  \mathcal{P} \equiv \{p, T, \rho, v, e, h, s, c, \mu, c_p, c_v, k, \beta\} \,.
\end{equation}
The set of the same quantities for the *stagnation* state, rather than the
static state, is denoted as $\mathcal{P}_0$.

Let the set of valid inputs for single-phase *static* fluid properties be
\begin{equation}
  \mathcal{A} \equiv \{ \{p, T\}, \{p, \rho\}, \{\rho, e\} \} \,,
\end{equation}
and the set of valid inputs for single-phase *stagnation* fluid properties be
\begin{equation}
  \mathcal{A}_0 \equiv \{ \{p, T, u\}, \{p, \rho, u\}, \{\rho, e, u\}, \{\rho, \rho u, \rho E\} \} \,.
\end{equation}
For single-phase vapor mixture fluid properties, the valid input sets are as follows:
\begin{equation}
  \mathcal{A}_{mix} \equiv \{ \{p, T, x_{NCG}\}, \{\rho, e, x_{NCG}\} \} \,,
\end{equation}
\begin{equation}
  \mathcal{A}_{0,mix} \equiv \{ \{p, T, x_{NCG}, u\}, \{\rho, e, x_{NCG}, u\},  \{\rho, \rho u, \rho E, x_{NCG}\} \} \,,
\end{equation}


The following table summarizes the valid input combinations for single-phase
fluid properties objects.
Note that
`TwoPhaseNCGFluidProperties` inherits from `TwoPhaseFluidProperties`, so the
column `TwoPhaseFluidProperties` is used to describe fluid properties classes
that derive from `TwoPhaseFluidProperties` but not `TwoPhaseNCGFluidProperties`.

| Base Class | Valid Input Combinations | Outputs |
| - | - | - |
| `SinglePhaseFluidProperties` | $\mathcal{I}\in\mathcal{A}$   | $\mathcal{P}$ |
|                              | $\mathcal{I}\in\mathcal{A}_0$ | $\mathcal{P}$, $\mathcal{P}_0$ |
| `VaporMixtureFluidProperties` | $\mathcal{I}\in\mathcal{A}_{mix}$   | $\mathcal{P}$ |
|                               | $\mathcal{I}\in\mathcal{A}_{0,mix}$ | $\mathcal{P}$, $\mathcal{P}_0$ |
| `TwoPhaseFluidProperties` | $\mathcal{I} = \{\}$     | $p_{crit}$ |
|                           | $\mathcal{I}\in\{p\}$       | $p_{crit}$, $T_{sat}$, $\Delta h_{\ell\rightarrow v}$ |
|                           | $\mathcal{I}\in\{T\}$       | $p_{crit}$, $p_{sat}$, $\Delta h_{\ell\rightarrow v}$ |
|                           | $\mathcal{I}\in\{p, T\}$    | $p_{crit}$, $\Delta h_{\ell\rightarrow v}$ |
|                           | $\mathcal{I}\in\mathcal{A}, \mathcal{I}\ne\{p, T\}$ | $p_{crit}$, $\mathcal{P}_\ell$, $\mathcal{P}_v$ |
|                           | $\mathcal{I}\in\mathcal{A}_0$ | $p_{crit}$, $\mathcal{P}_\ell$, $\mathcal{P}_v$, $\mathcal{P}_{0,\ell}$, $\mathcal{P}_{0,v}$ |
| `TwoPhaseNCGFluidProperties` | $\mathcal{I} = \{\}$     | $p_{crit}$ |
|                              | $\mathcal{I}\in\{p\}$       | $p_{crit}$, $T_{sat}$, $\Delta h_{\ell\rightarrow v}$ |
|                              | $\mathcal{I}\in\{T\}$       | $p_{crit}$, $p_{sat}$, $\Delta h_{\ell\rightarrow v}$ |
|                              | $\mathcal{I}\in\{p, T\}$    | $p_{crit}$, $\Delta h_{\ell\rightarrow v}$ |
|                              | $\mathcal{I}\in\mathcal{A}, \quad \mathcal{I}\ne\{p, T\}$ | $p_{crit}$, $\mathcal{P}_\ell$ |
|                              | $\mathcal{I}\in\mathcal{A}_0$ | $p_{crit}$, $\mathcal{P}_\ell$, $\mathcal{P}_{0,\ell}$ |
|                              | $\mathcal{I}\in\mathcal{A}_{mix}$   | $p_{crit}$, $\mathcal{P}_\ell$, $\mathcal{P}_v$ |
|                              | $\mathcal{I}\in\mathcal{A}_{0,mix}$ | $p_{crit}$, $\mathcal{P}_\ell$, $\mathcal{P}_v$, $\mathcal{P}_{0,\ell}$, $\mathcal{P}_{0,v}$ |

!syntax parameters /UserObjects/FluidPropertiesInterrogator

!syntax inputs /UserObjects/FluidPropertiesInterrogator

!syntax children /UserObjects/FluidPropertiesInterrogator
