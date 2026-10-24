# NASA7IdealGasFluidProperties

!syntax description /FluidProperties/NASA7IdealGasFluidProperties

This class implements an ideal gas whose heat capacity, enthalpy, and entropy depend on
temperature through the NASA 7-coefficient polynomials [!citep](mcbride1993) of a built-in
species. The species is selected with [!param](/FluidProperties/NASA7IdealGasFluidProperties/species):
H$_2$, O$_2$, H$_2$O, CH$_4$, CO, CO$_2$, or N$_2$. The data are those of GRI-Mech 3.0
[!citep](smith_grimech30) as distributed with Cantera 3.2.0 [!citep](cantera320); the polynomials,
the data, and their temperature ranges are described with the
[NASA 7-coefficient polynomial utilities](utils/FluidPropertiesUtils.md).

```
[FluidProperties]
  [steam]
    type = NASA7IdealGasFluidProperties
    species = H2O
  []
[]
```

## Properties

The equation of state is that of an ideal gas:

!equation
p v = R_s T \,, \qquad R_s = \frac{R}{M} \,,

where $p$ is pressure, $v$ is specific volume, $T$ is temperature, $R$ is the universal gas
constant, and $M$ is the molar mass of the species. With the dimensionless standard-state heat
capacity $c_p^\circ / R$, enthalpy $h^\circ / (R T)$, and entropy $s^\circ / R$ from the
polynomials, the specific enthalpy, internal energy, entropy, and heat capacities are

!equation
\begin{aligned}
h(T) &= R_s T \, \frac{h^\circ}{R T} \,, \\
e(T) &= h(T) - R_s T \,, \\
s(p, T) &= R_s \left( \frac{s^\circ}{R} - \ln \frac{p}{p^\circ} \right) , \\
c_p(T) &= R_s \, \frac{c_p^\circ}{R} \,, \\
c_v(T) &= c_p(T) - R_s \,,
\end{aligned}

where $p^\circ = 101325$ Pa is the reference pressure of the data. The ratio of specific heats is
$\gamma = c_p / c_v$, and the sound speed is $c = \sqrt{\gamma R_s T}$.

The enthalpy includes the enthalpy of formation of the species, and the entropy is the absolute
entropy. The enthalpies of different species therefore share a common reference, so a mixture
energy balance accounts for the heat of reaction when the composition changes by reaction. This
differs from [IdealGasFluidProperties.md], whose internal energy is $c_v T$ plus a user-supplied
reference value.

The derivatives of the properties are computed by forward-mode automatic differentiation of the
expressions above, so they are exact.

This class provides the properties from pressure and temperature, from temperature and specific
volume, from specific volume and internal energy, and from pressure and enthalpy, as well as the
pressure from enthalpy and entropy and the internal energy from pressure and density and from
specific volume and enthalpy. It does not provide viscosity or thermal conductivity, so it cannot
yet be used in flow models that need them, such as the gas-mixture flow channels of the thermal
hydraulics module. Mixtures of these gases are described by [IdealGasMixtureFluidProperties.md].

## Temperature from internal energy or enthalpy

The properties from specific volume and internal energy, and those from enthalpy, require the
temperature at which the internal energy or the enthalpy takes a given value. The temperature is
found with the 1D Newton solver of the [fluid properties utilities](utils/FluidPropertiesUtils.md),
whose relative tolerance and maximum number of iterations are set with
[!param](/FluidProperties/NASA7IdealGasFluidProperties/tolerance) and
[!param](/FluidProperties/NASA7IdealGasFluidProperties/max_newton_its). Two choices make this
inversion robust:

- The iteration starts at the upper end of the temperature range of the data, so this class has
  no initial-guess parameters. The
  internal energy and the enthalpy are convex functions of temperature over nearly all of that
  range, because the heat capacity increases with temperature, so the iterates approach the
  solution from above. An iteration started below the solution can overshoot far beyond the
  range, where the extrapolated polynomials eventually make the energy decrease with temperature
  and produce a second, spurious solution. For the built-in species, the solution is found up to
  at least 1.7 times the upper end of the range.
- The internal energy and the enthalpy include the enthalpy of formation, so they can be zero:
  the internal energy of N$_2$, for example, vanishes near 417 K. The Newton solver tests
  convergence relative to the target value, which close to zero would demand accuracy below
  roundoff. The equation is therefore offset by a constant that makes both of its sides close to
  $c_v T$ (or $c_p T$), which turns the test into one on the relative error in temperature.

At the midpoint temperature of the data, where the two polynomials do not match exactly, the same
internal energy or enthalpy is also reached slightly above the midpoint temperature, by less than
$3 \times 10^{-4}$ K for the built-in species. The inversion may return either temperature.

## Temperature range

The polynomials are fitted from 200 K to 3500 K, except for N$_2$, whose fit extends from 300 K to
5000 K. Outside its range, the polynomials of a species are still evaluated, and each such
evaluation is counted as a solution warning, which appears in the solution invalidity summary. For
example, N$_2$ at room temperature lies slightly below its range. A non-positive or not-a-number
temperature is flagged as an invalid solution.

!syntax parameters /FluidProperties/NASA7IdealGasFluidProperties

!syntax inputs /FluidProperties/NASA7IdealGasFluidProperties

!syntax children /FluidProperties/NASA7IdealGasFluidProperties

!bibtex bibliography
