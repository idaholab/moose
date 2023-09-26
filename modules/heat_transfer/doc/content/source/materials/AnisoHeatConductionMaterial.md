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

It is mandatory for the user to supply the parameter `thermal_conductivity` as a vector with nine element '$k_{11}$ $k_{21}$ $k_{31}$ $k_{12}$ $k_{22}$ $k_{32}$ $k_{13}$ $k_{23}$ $k_{33}$'. The thermal conductivity can be set as a function of temperature ($T$) by providing the parameters `thermal_conductivity_temperature_coefficient_function` ($\alpha$) and the `reference_temperature` ($T_{ref}$). The thermal conductivity as a function of temperature $k(T)$ is expressed as:

\begin{equation}
 k(T) = k_{o} (1 + \alpha (T - T_{ref}) )
\end{equation}

where $k_{o}$ is the thermal conductivity based on the user supplied vector. The specific heat capacity $C_p$ can be supplied either as a constant value or as a function using the parameter `specific_heat`.

It should be noted that in some analyses the thermal conductivity could potentially depend on other variables (e.g. solute concentration). The user should provide off diagonal Jacobian contributions in these analyses.

## Example Input Syntax

!listing /test/tests/heat_conduction_ortho/heat_conduction_ortho.i block=Materials/heat

!syntax parameters /Materials/AnisoHeatConductionMaterial

!syntax inputs /Materials/AnisoHeatConductionMaterial

!syntax children /Materials/AnisoHeatConductionMaterial

!bibtex bibliography
