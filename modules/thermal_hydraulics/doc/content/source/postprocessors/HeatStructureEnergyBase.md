# HeatStructureEnergyBase

Post-processors deriving from `HeatStructureEnergyBase` compute the total energy
$E_{tot}$ stored in a heat structure:
\begin{equation}\label{eq:hs_total_energy}
  E_{tot} = \sum\limits_i^N \int\limits_{V_i} \rho c_p T dV \,,
\end{equation}
where $N$ is the number of regions in the heat structure that are considered
(i.e., the size of the parameter `block`), $\rho$ is the density, $c_p$ is the isobaric
specific heat capacity, $T$ is the temperature, and $V_i$ is the spatial domain of region $i$.

Optionally, one can supply a reference temperature $T_0$ (which is zero by
default, yielding [eq:hs_total_energy]) to compute the energy *change*:
\begin{equation}\label{eq:hs_energy_change}
  \Delta E_{tot} = \sum\limits_i^N \int\limits_{V_i} \rho c_p (T - T_0) dV \,.
\end{equation}
