# Tungsten Thermal Properties Material

!syntax description /Materials/TungstenThermalPropertiesMaterial

`TungstenThermalPropertiesMaterial` provides tungsten's thermal conductivity, specific heat, and density as material properties based on temperature

Properties of tungsten are obtained from [!cite](milner2024space).

# Thermal conductivity
For temperature range: 1 ≤ T < 55

\[
k(T) = \left( A0 \cdot \left( \frac{T}{1000} \right)^N \right)
\Big/
\left( 1 + A1 \cdot \left( \frac{T}{1000} \right) + A2 \cdot \left( \frac{T}{1000} \right)^2 + A3 \cdot \left( \frac{T}{1000} \right)^3 \right)
\]

where:

- `k(T)` = **Thermal Conductivity** `${units W/mK}`
- `T` = **Temperature** `${units K}`

| Constant | Value      | Unit                |
|----------|------------|---------------------|
| N        | 8.740E-01  | `${units 1}`        |
| A0       | 7.348E+05  | `${units W/mK}`     |
| A1       | 2.544E+01  | `${units 1}`        |
| A2       | -8.304E+03 | `${units 1}`        |
| A3       | 1.180E+06  | `${units 1}`        |

For temperature range 55 ≤ T ≤ 3653

\[
k(T) = \left( B0 + B1 \cdot \left( \frac{T}{1000} \right) + B2 \cdot \left( \frac{T}{1000} \right)^2 + B3 \cdot \left( \frac{T}{1000} \right)^3 \right)
\Big/
\left( C0 + C1 \cdot \left( \frac{T}{1000} \right) + \left( \frac{T}{1000} \right)^2 \right)
\]

where:

- `k(T)` = **Thermal Conductivity** `${units W/mK}`
- `T` = **Temperature** `${units K}`

| Constant | Value       | Unit                |
|----------|-------------|---------------------|
| B0       | -3.679E+00  | `${units W/mK}`     |
| B1       | 1.181E+02   | `${units W/mK}`     |
| B2       | 5.879E+01   | `${units W/mK}`     |
| B3       | 2.867E+00   | `${units W/mK}`     |
| C0       | -2.052E-02  | `${units 1}`        |
| C1       | 4.741E-01   | `${units 1}`        |

# Specific Heat
For temperature range: 11 ≤ T < 293

\[
C_p(T) = \left[ A0 \cdot \left( \frac{T}{1000} \right)^N \right]
\Big/
\left( 1 + A1 \cdot \left( \frac{T}{1000} \right) + A2 \cdot \left( \frac{T}{1000} \right)^2 + A3 \cdot \left( \frac{T}{1000} \right)^3 \right)
\]

where:

- `C_p(T)` = **Specific Heat** `${units J/gK}`
- `T` = **Temperature** `${units K}`

| Constant | Value      | Unit                |
|----------|------------|---------------------|
| N        | 3.030E+00  | `${units 1}`        |
| A0       | 3.103E+02  | `${units J/gK}`     |
| A1       | -8.815E+00 | `${units 1}`        |
| A2       | 1.295E+02  | `${units 1}`        |
| A3       | 1.874E+03  | `${units 1}`        |

For temperature range: 293 ≤ T < 3700

\[
C_p(T) = B0 + B1 \cdot \left( \frac{T}{1000} \right) + B2 \cdot \left( \frac{T}{1000} \right)^2 + B3 \cdot \left( \frac{T}{1000} \right)^3 + \frac{B\_2}{\left( \frac{T}{1000} \right)^2}
\]

where:
- `C_p(T)` = **Specific Heat** `${units J/gK}`
- `T` = **Temperature** `${units K}`

| Constant | Value       | Unit            |
|----------|-------------|-----------------|
| B0       | 1.301E-01   | `${units J/gK}` |
| B1       | 2.225E-02   | `${units J/gK}` |
| B2       | -7.224E-03  | `${units J/gK}` |
| B3       | 3.539E-03   | `${units J/gK}` |
| B_2      | -3.061E-04  | `${units J/gK}` |

# Density
For temperature range: 5 ≤ T < 3600
\[
\rho(T) = \frac{\rho_{RT}}{\left(1 + \frac{dL}{L_0(T)} \times \frac{1}{100}\right)^3}
\]

where:
- `𝜌(T)` = **Density** `${units Kg/m^3}`
- `𝜌_{RT}(T)` = **Room temperature density** `${units Kg/m^3}`
- `T` = **Temperature** `${units K}`

and the thermal expansion `dL/L_0(T) [%]` is given by:

\[
\frac{dL}{L_0}(T) = A0 + A1 \cdot \left( \frac{T}{1000} \right) + A2 \cdot \left( \frac{T}{1000} \right)^2 + A3 \cdot \left( \frac{T}{1000} \right)^3
\]

For temperature range: 5 ≤ T < 294

| Constant | Value      | Unit |
|----------|------------|------|
| A0       | -8.529E-02 | [1]  |
| A1       | -9.915E-02 | [1]  |
| A2       | 2.257E+00  | [1]  |
| A3       | -3.157E+00 | [1]  |

For temperature range: 294 ≤ T ≤ 3600

| Constant | Value      | Unit |
|----------|------------|------|
| A0       | -1.400E-01 | [1]  |
| A1       | 4.869E-01  | [1]  |
| A2       | -3.056E-02 | [1]  |
| A3       | 2.234E-02  | [1]  |

!listing modules/solid_properties/test/tests/tungsten/tungsten_thermal_properties.i block=Materials

!syntax parameters /Materials/TungstenThermalPropertiesMaterial

!syntax inputs /Materials/TungstenThermalPropertiesMaterial

!syntax children /Materials/TungstenThermalPropertiesMaterial

!bibtex bibliography
