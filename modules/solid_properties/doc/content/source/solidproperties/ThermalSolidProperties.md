# ThermalSolidProperties

This is the base class for providing thermal solid properties
as a function of temperature.
This class defines functions to compute the following thermal properties,
which are all assumed to be functions of temperature only:

- `cp`: isobaric specific heat, $c_p$
- `e`: specific internal energy, $e$
- `k`: thermal conductivity, $k$
- `rho`: density, $\rho$

For each of these, the following methods are available, where `y` should be
replaced by the respective property name given in the list above:

```
Real y_from_T(const Real & T) const
```

provides the value $y(T)$ using a `Real` input value for $T$.

```
void y_from_T(const Real & T, Real & y, Real & dy_dT) const
```

provides the value $y(T)$ and its derivative $y'(T)$ using a `Real` input value for $T$.

```
ADReal y_from_T(const ADReal & T) const
```

provides the value $y(T)$ and its derivative $y'(T)$ using an `ADReal` input value for $T$.

Thus both AD and non-AD interfaces are available. Derived classes are only responsible
for overriding the non-AD interfaces for each property. The AD interfaces are
implemented by default by combining the two non-AD interfaces.

Note that the `e_from_T` interfaces should not be overridden because they have
generic implementations, and the non-AD interfaces are not even virtual.
Instead, derived classes must override the following:

```
Real cp_integral(const Real & T) const
```

which corresponds to the indefinite integral $C(T)$ of $c_p(T)$, minus the
constant of integration:

!equation
C(T) = \int c_p(T) dT \,.

Due to the definition of the isobaric specific heat capacity,

!equation
c_p \equiv \left.\frac{\partial e}{\partial T}\right|_v \,,

the specific internal energy can be expressed as

!equation
e(T) - e(T_0) = \int\limits_{T_0}^T c_p(T') dT' = C(T) - C(T_0) \,,

where $T_0$ is the temperature at which the specific internal energy is assumed
to be zero. This is a convention supplied by the user using the parameter
[!param](/SolidProperties/ThermalSS316Properties/T_zero_e). By default, this
is taken to be at standard temperature, 273.15 K. Note that this is important
for comparing specific internal energy values to external sources, which may
be based on different reference temperatures.

!bibtex bibliography
