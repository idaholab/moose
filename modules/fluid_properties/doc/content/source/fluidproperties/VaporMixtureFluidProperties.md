# VaporMixtureFluidProperties

`VaporMixtureFluidProperties` is a base class for gas mixture fluid properties
objects. Its main function is to provide interfaces for computing various properties
from different combinations of other properties and the mass fractions of its $n$ components.

## Properties

The following properties are considered in this class, where the "Name" column
gives the identifier for the property used in the available interfaces:

| Name | Symbol | Description |
| :- | :- | :- |
| `c` | $c$ | Speed of sound |
| `cp` | $c_p$ | Isobaric specific heat capacity |
| `cv` | $c_v$ | Isochoric specific heat capacity |
| `e` | $e$ | Specific internal energy |
| `k` | $k$ | Thermal conductivity |
| `mu` | $\mu$ | Dynamic viscosity |
| `p` | $p$ | Pressure |
| `rho` | $\rho$ | Density |
| `s` | $s$ | Specific entropy |
| `T` | $T$ | Temperature |
| `v` | $v$ | Specific volume |

Each mixture property lookup is of the form $f(a,b,\mathbf{x})$
where $f$ is the desired thermodynamic property and $a$ and $b$ are independent,
intensive thermodynamic properties, and $\mathbf{x}$ is a vector of $n-1$ mass fractions $\{x_2, \ldots, x_n\}$;
it does not include the mass fraction of the first component, since it can be determined
by the others using the partition of unity property: $\sum\limits_{i=1}^n x_i = 1$.
The corresponding function name is `fname_from_aname_bname`, where `fname`, `aname`,
and `bname` are the names in the table above, corresponding to $f$, $a$, and $b$,
respectively. The following table lists which
properties are available from various combinations of properties (e.g., "Yes"
in the column $(a,b)$ for the row $f$ denotes that the interface `fname_from_aname_bname`
is available):

| Name     | $(p,T)$ | $(v,e)$ | $(p,\rho)$ |
| :-       | -       | -       | -          |
| $c$      | Yes     | Yes     |            |
| $c_p$    | Yes     |         |            |
| $c_v$    | Yes     |         |            |
| $e$      | Yes     |         | Yes        |
| $k$      | Yes     |         |            |
| $\mu$    | Yes     |         |            |
| $p$      |         | Yes     |            |
| $\rho$   | Yes     |         |            |
| $s$      | Yes     |         |            |
| $T$      |         | Yes     |            |

Interfaces are also provided for getting derivatives of fluid properties with respect
to the input arguments. These interfaces are named the same as their non-derivative
counterparts, but have no return value but 3 additional (output) arguments,
corresponding to the property value and then the derivatives of each of the two input
arguments. For example, $\rho(p,T)$ has the interface `rho_from_p_T(p, T, x, rho, drho_dp, drho_dT, drho_dx)`,
where `drho_dp`, `drho_dT`, and `drho_dx` correspond to $(\partial\rho/\partial p)|_{T,\mathbf{x}}$,
$(\partial\rho/\partial T)|_{p,\mathbf{x}}$, and $(\partial\rho/\partial \mathbf{x})|_{p,T}$ respectively.

!alert note title=Automatic Differentiation
Fluid properties objects have interfaces for taking advantage of MOOSE's
Automatic Differentiation capability. See the example in the next section.

Additionally, the following interfaces are available:

- `getPrimaryFluidProperties()`: The primary component of the mixture.
- `getSecondaryFluidProperties(i)`: The secondary component $i$ of the mixture.
- `getNumberOfSecondaryVapors()`: Returns $n-1$
- `numberOfComponents()`: Returns $n$
- `primaryMassFraction(x_secondary)`: Computes $x_1$ from $\{x_2, \ldots, x_n\}$
- `mixtureMolarMass(molar_fractions, molar_masses)`: Computes the mixture molar mass from the molar fractions and molar masses of all components
- `massFractionsFromMolarFractions(molar_fractions, molar_masses)`: Converts molar fractions to mass fractions

The full list of available methods can be found in either the source code or the
[Modules Doxygen](https://mooseframework.inl.gov/docs/doxygen/modules/classes.html) page for each
FluidProperties class.
