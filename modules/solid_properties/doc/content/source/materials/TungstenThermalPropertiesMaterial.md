# Tungsten Thermal Properties Material

!syntax description /Materials/TungstenThermalPropertiesMaterial

This class provides tungsten's thermal conductivity, specific heat, and density as material properties based on temperature.

Properties of tungsten are obtained from [!cite](milner2024space).

# Thermal conductivity

For temperature range: 1 ≤ T < 55 K

\begin{equation}
k(T) =
\frac{A0 \cdot \left( \frac{T}{1000} \right)^N}{1 + A1 \cdot \left( \frac{T}{1000} \right) + A2 \cdot \left( \frac{T}{1000} \right)^2 + A3 \cdot \left( \frac{T}{1000} \right)^3}
\end{equation}

where:

- `k(T)` is **Thermal Conductivity `[W/m.K]`

- `T` is **Temperature `[K]`

| Constant | Value      | Unit                |
|----------|------------|---------------------|
| N        | 8.740E-01  | `[1]`               |
| A0       | 7.348E+05  | `[W/m.K]`           |
| A1       | 2.544E+01  | `[1]`               |
| A2       | -8.304E+03 | `[1]`               |
| A3       | 1.180E+06  | `[1]`               |

For temperature range 55 ≤ T ≤ 3653 K

\begin{equation}
k(T) = \frac{B0 + B1 \cdot \left( \frac{T}{1000} \right) + B2 \cdot \left( \frac{T}{1000} \right)^2 + B3 \cdot \left( \frac{T}{1000} \right)^3}{C0 + C1 \cdot \left( \frac{T}{1000} \right) + \left( \frac{T}{1000} \right)^2}
\end{equation}

where:

- `k(T)` is **Thermal Conductivity `[W/m.K]`

- `T` is **Temperature `[K]`

| Constant | Value       | Unit                |
|----------|-------------|---------------------|
| B0       | -3.679E+00  | `[W/m.K]`           |
| B1       | 1.181E+02   | `[W/m.K]`           |
| B2       | 5.879E+01   | `[W/m.K]`           |
| B3       | 2.867E+00   | `[W/m.K]`           |
| C0       | -2.052E-02  | `[1]`               |
| C1       | 4.741E-01   | `[1]`               |

# Specific Heat

For temperature range: 11 ≤ T < 293

\begin{equation}
C_p(T) =\frac{A0 \cdot \left( \frac{T}{1000} \right)^N}{1 + A1 \cdot \left( \frac{T}{1000} \right) + A2 \cdot \left( \frac{T}{1000} \right)^2 + A3 \cdot \left( \frac{T}{1000} \right)^3}
\end{equation}

where:

- `C`$_p$`(T)` is **Specific Heat `[J/g.K]`

- `T` is **Temperature `[K]`

| Constant | Value      | Unit                |
|----------|------------|---------------------|
| N        | 3.030E+00  | `[1]`               |
| A0       | 3.103E+02  | `[J/g.K]`           |
| A1       | -8.815E+00 | `[1]`               |
| A2       | 1.295E+02  | `[1]`               |
| A3       | 1.874E+03  | `[1]`               |

For temperature range: 293 ≤ T < 3700

\begin{equation}
C_p(T) = B0 + B1 \cdot \left( \frac{T}{1000} \right) + B2 \cdot \left( \frac{T}{1000} \right)^2 + B3 \cdot \left( \frac{T}{1000} \right)^3 + \frac{B_{-2}}{\left( \frac{T}{1000} \right)^2}
\end{equation}

where:

- `C`$_p$`(T)` is **Specific Heat `[J/g.K]`

- `T` is **Temperature `[K]`

| Constant | Value       | Unit           |
|----------|-------------|----------------|
| B0       | 1.301E-01   | `[J/g.K]`      |
| B1       | 2.225E-02   | `[J/g.K]`       |
| B2       | -7.224E-03  | `[J/g.K]`      |
| B3       | 3.539E-03   | `[J/g.K]`      |
| B$_{-2}$ | -3.061E-04  | `[J/g.K]`      |

# Density

For temperature range: 5 ≤ T < 3600
\begin{equation}
\rho(T) = \frac{\rho_{RT}}{\left(1 + \frac{dL}{L_0(T)} \times \frac{1}{100}\right)^3}
\end{equation}

where:

- `𝜌(T)` is **Density `[Kg/m`$^3$`]`

- `𝜌`$_{RT}$`(T)` is **Room temperature density `= 19250 [Kg/m`$^3$`]`

- `T` is **Temperature `[K]`

and the thermal expansion $dL/L_0(T)$ `[%]` is given by:

\begin{equation}
\frac{dL(T)}{L_0} = A0 + A1 \cdot \left( \frac{T}{1000} \right) + A2 \cdot \left( \frac{T}{1000} \right)^2 + A3 \cdot \left( \frac{T}{1000} \right)^3
\end{equation}

For temperature range: 5 ≤ T < 294

| Constant | Value      | Unit           |
|----------|------------|----------------|
| A0       | -8.529E-02 | `[1]`          |
| A1       | -9.915E-02 | `[1]`          |
| A2       | 2.257E+00  | `[1]`          |
| A3       | -3.157E+00 | `[1]`          |

For temperature range: 294 ≤ T ≤ 3600

| Constant | Value      | Unit           |
|----------|------------|----------------|
| A0       | -1.400E-01 | `[1]`          |
| A1       | 4.869E-01  | `[1]`          |
| A2       | -3.056E-02 | `[1]`          |
| A3       | 2.234E-02  | `[1]`          |

!syntax parameters /Materials/TungstenThermalPropertiesMaterial

!syntax inputs /Materials/TungstenThermalPropertiesMaterial

!syntax children /Materials/TungstenThermalPropertiesMaterial

!bibtex bibliography
