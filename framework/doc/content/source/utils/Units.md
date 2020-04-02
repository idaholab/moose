# MooseUnits

`MooseUnits` is a physical units parsing and conversion helper class.
A unit object can be constructed form a string (e.g. `N*m`, `eV/at`, `kg*(m/s)^2`, `1/s`, `m^-3`).
The unit parser supports the `*`,`/`, and `^` operators as well as parenthesis `(`, `)`.
The argument of the `^` operator is expected to be a positive or negative integer.

Upon parsing all units are resolved to a combination of the
[seven base SI units](https://en.wikipedia.org/wiki/SI_base_unit) (`m`, `g`,
`s`, `A`, `K`, `mol`, `cd`) with their respective exponents and a prefactor. A
canonical form is generated that permits unit comparisons to determine
conformity (i.e. whether two units can be converted between).

`MooseUnits` supports prefixing all units with a
[metric prefix](https://en.wikipedia.org/wiki/Metric_prefix) ranging from `Y` (yotta)
to `y` (yocto).

## Supported derived units

The following units are currently supported

| Symbol | Base units | Name | Description |
| - | - | - | - |
| `Ohm` | $\text{kg}\cdot \text{m}^{2}\cdot \text{s}^{-3}\cdot \text{A}^{-2}$ | Ohm | resistance, impedance, reactance|
| `atm` | $101325\, \text{kg}\cdot \text{m}^{-1}\cdot \text{s}^{-2}$ | Standard atmosphere | pressure |
| `eV` | $1.60218\cdot10^{-19}\, \text{kg}\cdot \text{m}^{2}\cdot \text{s}^{-2}$ | electron Volt | energy |
| `erg` | $10^{-7}\, \text{kg}\cdot \text{m}^{2}\cdot \text{s}^{-2}$ | Erg | energy |
| `degC` | $\text{K}$ | Celsius | temperature - As long as this unit stands alone the additive shift in the scale is taken into account. In all other cases this unit behaves like Kelvin |
| `degF` | $\frac59\,\text{K}$ | Fahrenheit | temperature - As long as this unit stands alone the additive shift in the scale is taken into account. In all other cases this unit behaves like Rankine |
| `degR` | $\frac59\,\text{K}$ | Rankine | temperature |
| `Ang` | $10^{-10}\, \text{m}$ | Angstrom | length |
| `Ang` | $10^{-10}\, \text{m}$ | Angstrom | length |
| `m` | $\text{m}$ | meter | length |
| `g` | $0.001\, \text{kg}$ | gram | mass |
| `s` | $\text{s}$ | second | time |
| `A` | $\text{A}$ | Ampere | electric current |
| `K` | $\text{K}$ | Kelvin | temperature |
| `mol` | $6.02214\cdot10^{23}\, \text{at}$ | mole | amount of substance|
| `cd` | $\text{cd}$ | candela | luminous intensity |
| `N` | $\text{kg}\cdot \text{m}\cdot \text{s}^{-2}$ | Newton | force, weight |
| `Pa` | $\text{kg}\cdot \text{m}^{-1}\cdot \text{s}^{-2}$ | Pascal | pressure, stress |
| `J` | $\text{kg}\cdot \text{m}^{2}\cdot \text{s}^{-2}$ | Joule | energy, work, heat |
| `W` | $\text{kg}\cdot \text{m}^{2}\cdot \text{s}^{-3}$ | Watt | power, radiant flux |
| `C` | $\text{A}\cdot \text{s}$ | Coulomb | electric charge |
| `V` | $\text{kg}\cdot \text{m}^{2}\cdot \text{s}^{-3}\cdot \text{A}^{-1}$ | Volt | voltage (electrical potential), emf |
| `F` | $\text{kg}^{-1}\cdot \text{m}^{-2}\cdot \text{s}^{4}\cdot \text{A}^{2}$ | Farad | capacitance |
| `S` | $\text{kg}^{-1}\cdot \text{m}^{-2}\cdot \text{s}^{3}\cdot \text{A}^{2}$ | Siemens | electrical conductance |
| `Wb` | $\text{kg}\cdot \text{m}^{2}\cdot \text{s}^{-2}\cdot \text{A}^{-1}$ | Weber | magnetic flux|
| `T` | $\text{kg}\cdot \text{s}^{-2}\cdot \text{A}^{-1}$ | Tesla | magnetic flux density |
| `H` | $\text{kg}\cdot \text{m}^{2}\cdot \text{s}^{-2}\cdot \text{A}^{-2}$ | Henry | inductance |
| `Ba` | $0.1\, \text{kg}\cdot \text{m}^{-1}\cdot \text{s}^{-2}$ | barye | Pressure |
| `dyn` | $10^{-5}\, \text{kg}\cdot \text{m}\cdot \text{s}^{-2}$ | dyne | force, weight |
| `ft` | $0.3048\, \text{m}$ | Foot | length |
| `in` | $0.0254\, \text{m}$ | Inch | length |
| `lb` | $0.453592\, \text{kg}$ | pound  | mass |
| `lbf` | $4.44822\, \text{kg}\cdot \text{m}\cdot \text{s}^{-2}$ | pound-force | force |
| `psi` | $6894.76\, \text{kg}\cdot \text{m}^{-1}\cdot \text{s}^{-2}$ | pound-force per square inch | pressure, stress |
| `BTU` | $1055.06\, \text{kg}\cdot \text{m}^{2}\cdot \text{s}^{-2}$ | ISO 31-4 British thermal unit | heat |
| `bar` | $10^5\, \text{kg}\cdot \text{m}^{-1}\cdot \text{s}^{-2}$ | Bar | pressure, stress |
| `h` | $3600\, \text{s}$ | hour | time |
| `day` | $86400\, \text{s}$ | day | time (`d` would create an ambiguity between centi-day and candela)|
| `l` | $0.001\, \text{m}^{3}$ | liter | volume |
| `u` | $1.66054\cdot10^{-27}\, \text{kg}^{3}$ | unified atomic mass unit | mass |
| `at` | $\text{at}$ | atom | single count of substance |

## Operators

Unit objects support the `*` and `/` operators to multiply and divide units
respectively. An overload of `std::pow` is provided to exponentiate units.

The `==` equal operator is implemented for comparison of two units and a unit
and a real number. It returns true if the two units are exactly identical
including the prefactor. A comparison that omits the prefactor is provided by
the `conformsTo` method.

A unit is considered equal to a real number if all its base
unit exponents are zero and the prefactor matches the real number.

# Dimension check

In addition to the `conformsTo` check a few `bool` member functions are provided
to check if the unit represents a given physical dimension.

- `isLength()`
- `isTime()`
- `isMass()`
- `isCurrent()`
- `isTemperature()`

## Output

`MooseUnits` objects can be output to streams using the `<<` operator. The output
consists of the prefactor and the SI base units with their respective exponents. Two
stream manipulators are provided to toggle between plain text and LaTeX formatted
output of the units.

## See also

Unit conversion using `MooseUnits` is available in input files through the
`${units ...}` [Brace Expression](input_syntax.md optional=True).
