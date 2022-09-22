# SinglePhaseFluidProperties

`SinglePhaseFluidProperties` is a base class for all single-phase fluid properties
objects. Its main function is to provide interfaces for computing various properties
from different combinations of other properties.

## Properties

The following properties are considered in this class, where the "Name" column
gives the identifier for the property used in the available interfaces:

| Name | Symbol | Description |
| :- | :- | :- |
| `beta` | $\beta$ | Volumetric expansion coefficient |
| `c` | $c$ | Speed of sound |
| `cp` | $c_p$ | Isobaric specific heat capacity |
| `cv` | $c_v$ | Isochoric specific heat capacity |
| `e` | $e$ | Specific internal energy |
| `g` | $g$ | Gibbs free energy |
| `gamma` | $\gamma$ | Ratio of specific heats, $\frac{c_p}{c_v}$ |
| `h` | $h$ | Specific enthalpy |
| `k` | $k$ | Thermal conductivity |
| `mu` | $\mu$ | Dynamic viscosity |
| `p` | $p$ | Pressure |
| `rho` | $\rho$ | Density |
| `s` | $s$ | Specific entropy |
| `T` | $T$ | Temperature |
| `v` | $v$ | Specific volume |

Because two independent, intensive thermodynamic properties define a
thermodynamic state of a pure fluid, interfaces in these objects are of the form $f(a,b)$
where $f$ is the desired thermodynamic property and $a$ and $b$ are independent,
intensive thermodynamic properties that define the thermodynamic state. The
corresponding function name is `fname_from_aname_bname`, where `fname`, `aname`,
and `bname` are the names in the table above, corresponding to $f$, $a$, and $b$,
respectively. The following table lists which
properties are available from various combinations of properties (e.g., "Yes"
in the column $(a,b)$ for the row $f$ denotes that the interface `fname_from_aname_bname`
is available):

| Name     | $(p,T)$ | $(v,e)$ | $(p,s)$ | $(p,h)$ | $(T,v)$ | $(v,h)$ | $(p,\rho)$ | $(\rho,T)$ | $(h,s)$ |
| :-       | -       | -       | -       | -       | -       | -       | -          |            |         |
| $\beta$  | Yes     |         |         |         |         |         |            |            |         |
| $c$      | Yes     | Yes     |         |         |         |         |            |            |         |
| $c_p$    | Yes     | Yes     |         |         |         |         |            |            |         |
| $c_v$    | Yes     | Yes     |         |         | Yes     |         |            |            |         |
| $e$      | Yes     |         |         |         | Yes     | Yes     | Yes        |            |         |
| $g$      |         | Yes     |         |         |         |         |            |            |         |
| $\gamma$ | Yes     | Yes     |         |         |         |         |            |            |         |
| $h$      | Yes     |         |         |         | Yes     |         |            |            |         |
| $k$      | Yes     | Yes     |         |         |         |         |            | Yes        |         |
| $\mu$    | Yes     | Yes     |         |         |         |         |            | Yes        |         |
| $p$      |         | Yes     |         |         | Yes     |         |            |            | Yes     |
| $\rho$   | Yes     |         | Yes     |         |         |         |            |            |         |
| $s$      | Yes     | Yes     |         | Yes     | Yes     |         |            |            |         |
| $T$      |         | Yes     |         | Yes     |         |         |            |            |         |
| $v$      | Yes     |         |         |         |         |         |            |            |         |

Interfaces are also provided for getting derivatives of fluid properties with respect
to the input arguments. These interfaces are named the same as their non-derivative
counterparts, but have no return value but 3 additional (output) arguments,
corresponding to the property value and then the derivatives of each of the two input
arguments. For example, $\rho(p,T)$ has the interface `rho_from_p_T(p, T, rho, drho_dp, drho_dT)`,
where `drho_dp` and `drho_dT` correspond to $(\partial\rho/\partial p)|_T$ and
$(\partial\rho/\partial T)|_p$, respectively.

!alert note title=Automatic Differentiation
Fluid properties objects have interfaces for taking advantage of MOOSE's
Automatic Differentiation capability. See the example in the next section.

Additionally, the following interfaces are available:

- `fluidName()`: The fluid name.
- `molarMass()`: The fluid's molar mass (kg/mol).

The full list of available methods can be found in either the source code or the
[Modules Doxygen](http://mooseframework.org/docs/doxygen/modules/classes.html) page for each
FluidProperties class.

## Default Analytical Fluid Properties Relations

`SinglePhaseFluidProperties` provides a number of default implementations for some fluid properties where
analytical relations hold for all single phase fluid properties. Some of these fluid properties
are also implemented along with their derivatives with regards to the input variables, when
these derivatives can also be analytically described. Relevant automatic differentiation (AD)
implementations are also provided through a `macro` to avoid duplicated code.

The full list of available methods can be found in either the source code or the
[Doxygen](https://mooseframework.inl.gov/docs/doxygen/modules/classSinglePhaseFluidProperties.html) page.

## Variable Set Conversions

Different fluid applications may require different variable sets, such as (pressure, temperature)
or (specific volume, specific internal energy), depending on the flow regimes of interest and relatedly
the numerical discretization. Fluid properties are not necessarily implemented or known for all variable sets,
so conversions from one variable set to another can be helpful.

For many fluids, analytical closures for these conversions are not known, so `SinglePhaseFluidProperties`
defines several routines for iteratively converting from one variable set
to another. This leverages the [numerical inversion methods utilities](utils/FluidPropertiesUtils.md).
Notably, the following routines are provided:

!listing modules/fluid_properties/include/userobjects/SinglePhaseFluidProperties.h start=p_T_from_v_e end=) const include-end=true

!listing modules/fluid_properties/include/userobjects/SinglePhaseFluidProperties.h start=p_T_from_v_h end=) const include-end=true

!listing modules/fluid_properties/include/userobjects/SinglePhaseFluidProperties.h start=p_T_from_h_s end=) const include-end=true

These routines may then be used to convert from one variable set to another before obtaining the desired
fluid property. For example, this routine converts (pressure, temperature) to (specific volume, specific energy)
to compute entropy.

!listing modules/fluid_properties/src/userobjects/SinglePhaseFluidProperties.C start=SinglePhaseFluidProperties::s_from_p_T end=} include-end=true
