# Tungsten Thermal Properties Material

!syntax description /Materials/TungstenThermalPropertiesMaterial

This class provides tungsten's thermal conductivity, specific heat, and density as material properties as a function of temperature based on the models listed in [!cite](milner2024space).

# Thermal conductivity

For temperature range: 1 ≤ T < 55 K, the thermal conductivity is defined as

\begin{equation} \label{eq:thermal_conductivity_low}
k(T) =
\frac{A0 \cdot \left( \frac{T}{1000} \right)^N}{1 + A1 \cdot \left( \frac{T}{1000} \right) + A2 \cdot \left( \frac{T}{1000} \right)^2 + A3 \cdot \left( \frac{T}{1000} \right)^3},
\end{equation}

where:

- $k(T)$ is the thermal conductivity `[W/m.K]`,

- $T$ is the temperature `[K]`,

and the model parameters values provided in [tab:thermal_conductivity_parameters_low].

!table id=tab:thermal_conductivity_parameters_low caption=Parameters values for the thermal conductivity model from [eq:thermal_conductivity_low].
| Constant | Value      | Unit                |
|----------|------------|---------------------|
| N        | 8.740E-01  | `[-]`               |
| A0       | 7.348E+05  | `[W/m.K]`           |
| A1       | 2.544E+01  | `[-]`               |
| A2       | -8.304E+03 | `[-]`               |
| A3       | 1.180E+06  | `[-]`               |

For temperature range 55 ≤ $T$ ≤ 3653 K, the thermal conductivity is defined as

\begin{equation} \label{eq:thermal_conductivity_high}
k(T) = \frac{B0 + B1 \cdot \left( \frac{T}{1000} \right) + B2 \cdot \left( \frac{T}{1000} \right)^2 + B3 \cdot \left( \frac{T}{1000} \right)^3}{C0 + C1 \cdot \left( \frac{T}{1000} \right) + \left( \frac{T}{1000} \right)^2},
\end{equation}

where:

- $k(T)$ is the thermal conductivity `[W/m.K]`,

- $T$ is the temperature `[K]`,

and the model parameters values provided in [tab:thermal_conductivity_parameters_high].

!table id=tab:thermal_conductivity_parameters_high caption=Parameters values for the thermal conductivity model from [eq:thermal_conductivity_high].
| Constant | Value       | Unit                |
|----------|-------------|---------------------|
| B0       | -3.679E+00  | `[W/m.K]`           |
| B1       | 1.181E+02   | `[W/m.K]`           |
| B2       | 5.879E+01   | `[W/m.K]`           |
| B3       | 2.867E+00   | `[W/m.K]`           |
| C0       | -2.052E-02  | `[-]`               |
| C1       | 4.741E-01   | `[-]`               |

# Specific Heat

For temperature range: 11 ≤ $T$ < 293 K, the specific heat  is described as:

\begin{equation} \label{eq:specific_heat_low}
C_p(T) =\frac{A0 \cdot \left( \frac{T}{1000} \right)^N}{1 + A1 \cdot \left( \frac{T}{1000} \right) + A2 \cdot \left( \frac{T}{1000} \right)^2 + A3 \cdot \left( \frac{T}{1000} \right)^3},
\end{equation}

where:

- $C_p$(T)$ is the specific heat `[J/g.K]`,

- $T$ is the temperature `[K]`,

and the model parameters values provided in [tab:specific_heat_parameters_low].

!table id=tab:specific_heat_parameters_low caption=Parameters values for the specific heat model from [eq:specific_heat_low].
| Constant | Value      | Unit                |
|----------|------------|---------------------|
| N        | 3.030E+00  | `[-]`               |
| A0       | 3.103E+02  | `[J/g.K]`           |
| A1       | -8.815E+00 | `[-]`               |
| A2       | 1.295E+02  | `[-]`               |
| A3       | 1.874E+03  | `[-]`               |

For temperature range: 293 ≤ $T$ < 3700 K, the specific heat is defined as

\begin{equation} \label{eq:specific_heat_high}
C_p(T) = B0 + B1 \cdot \left( \frac{T}{1000} \right) + B2 \cdot \left( \frac{T}{1000} \right)^2 + B3 \cdot \left( \frac{T}{1000} \right)^3 + \frac{B_{-2}}{\left( \frac{T}{1000} \right)^2}
\end{equation}

where:

- $C_p$(T)$ is the specific heat `[J/g.K]`,

- $T$ is the temperature `[K]`,

and the model parameters values provided in [tab:specific_heat_parameters_high].

!table id=tab:specific_heat_parameters_high caption=Parameters values for the specific heat model from [eq:specific_heat_high].
| Constant | Value       | Unit           |
|----------|-------------|----------------|
| B0       | 1.301E-01   | `[J/g.K]`      |
| B1       | 2.225E-02   | `[J/g.K]`      |
| B2       | -7.224E-03  | `[J/g.K]`      |
| B3       | 3.539E-03   | `[J/g.K]`      |
| B$_{-2}$ | -3.061E-04  | `[J/g.K]`      |

# Density

For the temperature range: 5 ≤ $T$ < 3600 K, the density is defined as
\begin{equation} \label{eq:density}
\rho(T) = \frac{\rho_{RT}}{\left(1 + \frac{dL}{L_0(T)} \times \frac{1}{100}\right)^3},
\end{equation}

where:

- $\rho(T)$ is the density `[Kg/m`$^3$`]`,

- $\rho_{RT}(T)$ is the room temperature density, set to 19250 `[Kg/m`$^3$`]`,

- $T$ is the temperature `[K]`,

and the thermal expansion $dL/L_0(T)$ `[%]` is given by:

\begin{equation} \label{eq:thermal_expansion}
\frac{dL(T)}{L_0} = A0 + A1 \cdot \left( \frac{T}{1000} \right) + A2 \cdot \left( \frac{T}{1000} \right)^2 + A3 \cdot \left( \frac{T}{1000} \right)^3.
\end{equation}

The values of the model parameters in [eq:thermal_expansion] are provided in [tab:thermal_expansion_parameters_low] for low temperatures (5 ≤ $T$ < 294 K), and [tab:thermal_expansion_parameters_high] for high temperatures (294 ≤ $T$ ≤ 3600 K).

!table id=tab:thermal_expansion_parameters_low caption=Parameters values for the thermal expansion model from [eq:thermal_expansion] for 5 ≤ $T$ < 294 K.
| Constant | Value      | Unit           |
|----------|------------|----------------|
| A0       | -8.529E-02 | `[-]`          |
| A1       | -9.915E-02 | `[-]`          |
| A2       | 2.257E+00  | `[-]`          |
| A3       | -3.157E+00 | `[-]`          |

!table id=tab:thermal_expansion_parameters_high caption=Parameters values for the thermal expansion model from [eq:thermal_expansion] for 294 ≤ $T$ ≤ 3600 K.
| Constant | Value      | Unit           |
|----------|------------|----------------|
| A0       | -1.400E-01 | `[-]`          |
| A1       | 4.869E-01  | `[-]`          |
| A2       | -3.056E-02 | `[-]`          |
| A3       | 2.234E-02  | `[-]`          |

!syntax parameters /Materials/TungstenThermalPropertiesMaterial

!syntax inputs /Materials/TungstenThermalPropertiesMaterial

!syntax children /Materials/TungstenThermalPropertiesMaterial

!bibtex bibliography
