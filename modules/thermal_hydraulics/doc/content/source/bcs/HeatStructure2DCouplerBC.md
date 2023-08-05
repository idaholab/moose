# HeatStructure2DCouplerBC

!syntax description /BCs/HeatStructure2DCouplerBC

The contribution to the residual $R_i$ for the weak form of the energy equation is computed as:

\begin{equation}
R_i = (\psi_i, h(t, \vec{x}) (T - T_{coupled} ) A_{fraction} ) \quad \forall \psi_i,
\end{equation}

where $\psi_i$ are the test functions, $h$ is the heat transfer coefficient provided by a [Function](syntax/Functions/index.md),
$T$ is the temperature variable on one of the components, $T_{coupled}$ the temperature variable on the other
component, and $A_{fraction}$ the area of contact between the two components.

!alert note
In THM, most boundary conditions are added automatically by components. This boundary condition is created by the
[HeatStructure2DCoupler.md] to couple the temperature variable on both sides of the boundaries between the heat structure
components. It is added once for each heat structure involved.

!syntax parameters /BCs/HeatStructure2DCouplerBC

!syntax inputs /BCs/HeatStructure2DCouplerBC

!syntax children /BCs/HeatStructure2DCouplerBC
