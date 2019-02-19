!syntax description /Postprocessors/HeatStructureEnergy

This post-processor computes the total energy $E_{tot}$ stored in a heat structure:
\begin{equation}
  E_{tot} = \sum\limits_i^N \int\limits_{V_i} \rho c_p T dV \,,
\end{equation}
where $N$ is the number of regions in the heat structure that are considered
(i.e., the size of the parameter `block`), $\rho$ is the density, $c_p$ is the isobaric
specific heat capacity, $T$ is the temperature, and $V_i$ is the spatial domain of region $i$.

!syntax parameters /Postprocessors/HeatStructureEnergy

!syntax inputs /Postprocessors/HeatStructureEnergy

!syntax children /Postprocessors/HeatStructureEnergy
