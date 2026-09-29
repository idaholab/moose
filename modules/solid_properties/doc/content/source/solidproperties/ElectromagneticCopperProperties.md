# ElectromagneticCopperProperties

!syntax description /SolidProperties/ElectromagneticCopperProperties

## Description

This userobject provides electromagnetic properties for oxygen-free high-conductivity (OFHC) copper
as a function of temperature using correlations from NIST Monograph 177 [!cite](nist_mono_177).
OFHC copper is used as a stabilizer/conductor in superconducting magnets such as those in the ITER central solenoid.

!include solid_properties_units.md

### Electrical Resistivity

Electrical resistivity is computed from the NIST zero-field correlation (equation 8-1 in [!cite](nist_mono_177)),
which uses a three-component model:

\begin{equation}
\rho(T) = \rho_0 + \rho_i(T) + \rho_{i0}(T)
\end{equation}

where:

- $\rho_0$ is the residual resistivity, computed from the residual resistivity ratio (RRR):
  \begin{equation}
  \rho_0 = \frac{1.553 \times 10^{-8}}{RRR} \quad [\Omega \cdot m]
  \end{equation}
  RRR characterizes sample purity. Higher RRR indicates higher purity and lower residual resistivity.
  The default RRR is 100.

- $\rho_i(T)$ is the ideal (lattice/phonon) resistivity:
  \begin{equation}
  \rho_i(T) = \frac{P_1 T^{P_2}}{1 + P_1 P_3 T^{P_2-P_4} \exp\left[-(P_5/T)^{P_6}\right]} + \rho_c
  \end{equation}
  with coefficients $P_1 = 1.171 \times 10^{-17}$, $P_2 = 4.49$, $P_3 = 3.841 \times 10^{10}$,
  $P_4 = 1.14$, $P_5 = 50$, $P_6 = 6.428$, and $\rho_c = 0$ for copper.

- $\rho_{i0}(T)$ is the deviation from Matthiessen's rule:
  \begin{equation}
  \rho_{i0}(T) = \frac{P_7 \rho_i(T) \rho_0}{\rho_i(T) + \rho_0}
  \end{equation}
  with coefficient $P_7 = 0.4531$.

The NIST correlation states that the equation reproduces data within the estimated accuracy
of the data for RRR values from 1 to 1000 over the temperature range 2 to 1000 K [!cite](nist_mono_177).

### Electrical Conductivity

Electrical conductivity is the reciprocal of resistivity:

\begin{equation}
\sigma(T) = \frac{1}{\rho(T)} \quad [S/m]
\end{equation}

The derivative is computed analytically using the chain rule:

\begin{equation}
\frac{d\sigma}{dT} = -\frac{1}{\rho^2} \frac{d\rho}{dT}
\end{equation}

### Magnetic Permeability

Copper is non-magnetic (diamagnetic with negligible susceptibility), so the magnetic permeability
is the permeability of free space:

\begin{equation}
\mu = \mu_0 = 4\pi \times 10^{-7} = 1.25663706212 \times 10^{-6} \quad [H/m]
\end{equation}

This property is constant and independent of temperature.

## Range of Validity

The electrical properties are valid for 2 K $\le$ T $\le$ 900 K (-271 °C to 627 °C).
This range is broader than the thermal and mechanical properties (4-300 K) to accommodate
high-temperature electromagnetic simulations.

!syntax parameters /SolidProperties/ElectromagneticCopperProperties

!syntax inputs /SolidProperties/ElectromagneticCopperProperties

!syntax children /SolidProperties/ElectromagneticCopperProperties

!bibtex bibliography
