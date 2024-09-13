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
implemented by default by combining the two non-AD interfaces. Also note that
the `void e_from_T(const Real & T, Real & e, Real & de_dT) const` interface is
not virtual and has a definition already, obtained by taking advantage of the
definition of isobaric specific heat capacity,

!equation
c_p \equiv \left.\frac{\partial e}{\partial T}\right|_v \,.

Therefore this interface should not and cannot be overridden in child classes.
Note that the above definition for $c_p$ can also give a definition for $e(T)$:

!equation
e(T) - e(T_0) = \int\limits_{T_0}^T c_p(T') dT' \,.

Note that this requires a decision for a reference temperature $T_0$, which may
be taken to be 0 K (absolute zero), or it may be any other value. This is
important to note when comparing to external property tables, which each may use
a particular reference value.

!bibtex bibliography
