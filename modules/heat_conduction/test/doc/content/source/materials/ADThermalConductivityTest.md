# ADThermalConductivity

## Description

`ADThermalConductivity` provides a test thermal conductivity that is used to
verify the correct implementation of automatic differentiation in `ADHeatConduction`,
and is of the form,
\begin{equation}
  k = c * T,
\end{equation}
where $k$ is the thermal conductivity, $c$ is some variable, and $T$ is temperature.

!syntax parameters /ADMaterials/ADThermalConductivity

!syntax inputs /ADMaterials/ADThermalConductivity

!syntax children /ADMaterials/ADThermalConductivity

!bibtex bibliography
