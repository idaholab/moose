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
volume, and the internal energy from pressure and density. It does not provide viscosity or
thermal conductivity.

## Temperature range

The polynomials are fitted from 200 K to 3500 K, except for N$_2$, whose fit extends from 300 K to
5000 K. Outside its range, the polynomials of a species are still evaluated, and each such
evaluation is counted as a solution warning, which appears in the solution invalidity summary. For
example, N$_2$ at room temperature lies slightly below its range. A non-positive temperature is
flagged as an invalid solution.

!syntax parameters /FluidProperties/NASA7IdealGasFluidProperties

!syntax inputs /FluidProperties/NASA7IdealGasFluidProperties

!syntax children /FluidProperties/NASA7IdealGasFluidProperties

!bibtex bibliography
