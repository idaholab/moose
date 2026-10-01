# MechanicalCopperProperties

!syntax description /SolidProperties/MechanicalCopperProperties

## Description

This userobject provides mechanical properties for oxygen-free high-conductivity (OFHC) copper as a function of temperature. Young's modulus and Poisson's ratio are from [!cite](nist_mono_177), and the coefficient of thermal expansion is from [!cite](nist_copper).

!include solid_properties_units.md

### Young's Modulus

Young's modulus is given by the polynomial correlation:

\begin{equation}
E(T) = 10^9 \times (137 - 1.27 \times 10^{-4} T^2) \quad \text{[Pa]}
\end{equation}

where the factor $10^9$ converts from GPa to Pa.

### Poisson's Ratio

Poisson's ratio follows a simple quadratic temperature dependence:

\begin{equation}
\nu(T) = 0.339 + 7.03 \times 10^{-8} T^2
\end{equation}

### Coefficient of Thermal Expansion

The thermal expansion coefficient uses a logarithmic polynomial correlation from the NIST Cryogenic Materials Database:

\begin{equation}
\log_{10}(\alpha \times 10^6) = \sum_{i=0}^{6} c_i [\log_{10}(T)]^i
\end{equation}

where the coefficients are:

| i | $c_i$ |
|---|---|
| 0 | -17.9081289 |
| 1 | 67.131914 |
| 2 | -118.809316 |
| 3 | 109.9845997 |
| 4 | -53.8696089 |
| 5 | 13.30247491 |
| 6 | -1.30843441 |

This logarithmic form provides accurate representation of thermal expansion behavior over a wide temperature range, particularly at cryogenic temperatures where thermal expansion is very small.

## Range of Validity

All properties are valid for temperatures from 4 K to 300 K, covering cryogenic to room temperature conditions relevant for superconducting magnet applications.

## Example Input

```
[SolidProperties]
  [copper_mechanical]
    type = MechanicalCopperProperties
  []
[]

[Materials]
  [copper_props]
    type = MechanicalSolidPropertiesMaterial
    temperature = T
    sp = copper_mechanical
  []
[]
```

!syntax parameters /SolidProperties/MechanicalCopperProperties

!syntax inputs /SolidProperties/MechanicalCopperProperties

!syntax children /SolidProperties/MechanicalCopperProperties

!bibtex bibliography
