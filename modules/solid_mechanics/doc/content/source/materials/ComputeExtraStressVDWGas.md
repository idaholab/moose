# Compute Extra Stress van der Waals Gas

!syntax description /Materials/ComputeExtraStressVDWGas

## Description

The class `ComputeExtraStressVDWGas` adds an additional hydrostatic stress term, ($\sigma^g_{ij}$), to
the residual calculation after the constitutive model calculation of the stress, as shown in
[eq:extra_stress_vdwgas].  This additional hydrostatic stress represents to the pressure
exerted by a van der Waals gas.


The diagonal components of the stress tensor are given by
\begin{equation}
  \label{eq:extra_stress_vdwgas}
  \sigma^g_{ii} = -\frac{kT}{\left(\frac{V_a}{c_g} - b \right) E^* }
\end{equation}
where $k$ is Boltzmann's constant, $T$ is the temperature, $V_a$ is the atomic volume of the lattice
atoms in the solid surrounding the gas phase, $c_g$ is the local gas atomic fraction (relative to
the surrounding solid), $b$ is the van der Waals gas hard-sphere exclusion volume, and $E^* $ is
an optional scale factor for non-dimensionalization. The off-diagonal components are
$\sigma^g_{ij} = 0$ for $i \neq j$.

## Example Input File Syntax

!listing modules/combined/test/tests/surface_tension_KKS/surface_tension_VDWgas.i block=Materials/gas_pressure

!syntax parameters /Materials/ComputeExtraStressVDWGas

!syntax inputs /Materials/ComputeExtraStressVDWGas

!syntax children /Materials/ComputeExtraStressVDWGas
