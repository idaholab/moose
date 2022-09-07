# AnisoHeatConductionMaterial

!syntax description /Materials/AnisoHeatConductionMaterial

## Description

The material model AnisoHeatConductionMaterial is used to model a material with anisotropic thermal properties. The thermal conductivity $k$ is represented as a rank two tensor:
\begin{equation}
  \begin{bmatrix}
  k_{11} & k_{12} & k_{13}\\
  k_{21} & k_{22} & k_{23}\\
  k_{31} & k_{32} & k_{33}\\
  \end{bmatrix}
  \label{eq:aeqn}
\end{equation}

The user can provide the thermal conductivity by setting the parameter `thermal_conductivity` as a vector with nine element '$k_{11}$ $k_{21}$ $k_{31}$ $k_{12}$ $k_{22}$ $k_{32}$ $k_{13}$ $k_{23}$ $k_{33}$'. If the parameter `thermal_conductivity` is supplied by the user, the thermal conductivity can be set as a function of temperature ($T$) by providing the parameters `thermal_conductivity_temperature_coefficient_function` ($\alpha$) and the `reference_temperature` ($T_{ref}$). The thermal conductivity as a function of temperature $k(T)$ is then expressed as:

\begin{equation}
 k(T) = k_{o} (1 + \alpha (T - T_{ref}) )
\end{equation}

where $k_{o}$ is the thermal conductivity based on the user supplied vector.


Another way, by which the user can provide thermal conductivity, is by providing the indivifual components `k_11`, `k_22` and `k_33` which can be either a constant value or a function of temperature.  Note that if `k_22` or `k_33` are not provided then their values will be set equal to that of `k_11`. Of the three parameters `k_11`, `k_22` and `k_33`, `k_11` is mandatory and the latter two are optional.

The specific heat capacity $C_p$ can be supplied either as a constant value or as a function using the parameter `specific_heat`.

It should be noted that in some analyses the thermal conductivity could potentially depend on other variables (e.g. solute concentration). The user should provide off diagonal Jacobian contributions in these analyses.

## Example Input Syntax

!listing /test/tests/heat_conduction_ortho/heat_conduction_ortho.i block=Materials/heat

!listing /test/tests/heat_conduction_ortho/heat_conduction_ortho_components.i block=Materials/heat

!syntax parameters /Materials/AnisoHeatConductionMaterial

!syntax inputs /Materials/AnisoHeatConductionMaterial

!syntax children /Materials/AnisoHeatConductionMaterial

!bibtex bibliography
