# GapFluxModelRadiation

!syntax description /UserObjects/GapFluxModelRadiation

## Description

`GapFluxModelRadiation` computes a radiative heat flux across a gap following the
diffusion approximation of radiation physics. This user object must be used in
combination with [ModularGapConductanceConstraint](ModularGapConductanceConstraint.md).

The heat flux across the gap is given by the classical expression
\begin{equation}
  \label{eqn:radiationHeatFlux}
  q_r = \sigma F_e \left( T_s^4 - T_f^4 \right) \sim h_r \left(T_s - T_f \right)
\end{equation}
where $\sigma$ is the Stephan-Boltzmann constant, $F_e$ is an emissivity function,
$T_s$ is the surface temperature, $T_f$ is the farfield temperature, and $h_r$
is the radiant gap conductance. This expression can be rearranged to solve for $h_r$:
\begin{equation}
  h_r = \sigma F_e \frac{\left( T_s^4 - T_f^4 \right)}{\left( T_s - T_f \right)}
\end{equation}
which reduces to
\begin{equation}
  \label{eqn:radiantGapConductance}
  h_r = \sigma F_e \left( T_s^2 + T_f^2 \right) \left( T_s + T_f \right).
\end{equation}

If the coordinate system type is Cartesian, the emissivity is computed using an
infinite parallel plate approximation given by
\begin{equation}
  \label{eqn:emissivityFunction}
  F_e = \frac{1}{\left( 1/e_s + 1/e_f -1 \right)}
\end{equation}
where $e_s$ and $e_f$ are the near surface and farfield emissivity values,
respectively. The primary and secondary surface emissivity values can be assigned
arbitrarily to $e_s$ and $e_f$. For an axisymmetric coordinate system, the
emissivity is computed using the same formula as that given in
[FVInfiniteCylinderRadiativeBC.md]:

\begin{equation}
\frac{e_s e_f r_f}{e_f r_f + e_s r_s \left(1 - e_f\right)}
\end{equation}

Here the $s$ subscript should correspond to whichever surface (secondary or
primary) has the smaller radius, and the $f$ subscript should correspond to
whichever surface has the larger radius.

## Example Input File Syntax

!listing modules/heat_conduction/test/tests/gap_heat_transfer_mortar/modular_gap_heat_transfer_mortar_displaced_radiation.i block=UserObjects/radiation

`GapFluxModelRadiation` must be used in conjunction with the modular gap conductance
constraint as shown below:

!listing modules/heat_conduction/test/tests/gap_heat_transfer_mortar/modular_gap_heat_transfer_mortar_displaced_radiation.i block=Constraints/ced

!syntax parameters /UserObjects/GapFluxModelRadiation

!syntax inputs /UserObjects/GapFluxModelRadiation

!syntax children /UserObjects/GapFluxModelRadiation

!bibtex bibliography
