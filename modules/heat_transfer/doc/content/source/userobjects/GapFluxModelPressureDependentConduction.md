# GapFluxModelPressureDependentConduction

!syntax description /UserObjects/GapFluxModelPressureDependentConduction

## Description

`GapFluxModelPressureDependentConduction` computes a conductive heat flux across
a closed gap between two solid bodies as a function of the normal mechanical pressure
at the interface. The normal contact pressure is included in this calculation as
a Lagrange multiplier associated with a lower-dimensional domain. This class
requires the use of [ModularGapConductanceConstraint](ModularGapConductanceConstraint.md).

The thermal conductance of the interface is calculated as
\begin{equation}
  \label{eq:pressureDepConductivity}
  C_T = \alpha k_{harm} \frac{P}{H_{harm}}
\end{equation}
where $\alpha$ is a scaling or fitting parameter, k$_{harm}$ is the harmonic mean
of the thermal conductivities, P is the contact pressure, and H$_{harm}$ is the
harmonic mean of the material hardness, following [!citep](cincotti2007modeling).
The harmonic mean of the thermal conductivities is given as
\begin{equation}
  \label{eq:harmonicMean}
  k_{harm} = \frac{2 k_1 k_2}{k_1 + k_2}
\end{equation}
where k$_1$ and k$_2$ are the thermal conductivities of the two materials on either
side of the closed gap interface. The harmonic mean of the hardness values is
calculated in a similar fashion.

### Analytical Solution

Using this heat flux object alone, the temperature of the hotter material at the
interface, T$^h_{int}$, is given by the analytical expression
\begin{equation}
T^h_{int} = \frac{T^c_{BC}C_T k_c + T^h_{BC} k_h \left(k_c +C_T \right)}{k_h (k_c + C_T) + k_c C_T}
\end{equation}
and the temperature of the cooler material at the interface, T$^c_{int}$, is
\begin{equation}
T^c_{int} = \frac{T^h_{int} C_T + T^c_{BC} k_c}{k_c + C_T}
\end{equation}
where T$^h_{BC}$ and T$^c_{BC}$ are the prescribed hot and cool temperature boundary
conditions, respectively, k$_h$ and k$_c$ are the thermal conductivities of the
materials associated with the hot and cool temperatures, and C$_T$ is the thermal
conduction at the interface as given by [eq:pressureDepConductivity].
Note that these expressions were derived assuming no deformation and unit thickness
of both materials in the direction of the temperature gradient.

## Example Input File Syntax

!listing modules/heat_conduction/test/tests/gap_heat_transfer_mortar/closed_gap_prescribed_pressure.i block=UserObjects/closed

`GapFluxModelPressureDependentConduction` must be used in conjunction with the modular gap conductance
constraint as shown below:

!listing modules/heat_conduction/test/tests/gap_heat_transfer_mortar/closed_gap_prescribed_pressure.i block=Constraints/thermal_contact

!syntax parameters /UserObjects/GapFluxModelPressureDependentConduction

!syntax inputs /UserObjects/GapFluxModelPressureDependentConduction

!syntax children /UserObjects/GapFluxModelPressureDependentConduction

!bibtex bibliography
