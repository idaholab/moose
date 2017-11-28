# ComputeCrackPropagationHeatEnergy

Calculate crack propagation heat source:
$$
- \frac{d\Psi}{dc} \cdot \frac{dc}{dt}
$$
where $\Psi$ is the free energy of the phase-field damage model:
$$
\Psi = (1 - c)^2 \cdot G_{0,+} + G_{0,-}
$$
$c$ is the order parameter for damage, continuous between 0 and 1
0 represents no damage, 1 represents fully cracked
$G_{0,+}$ and $G_{0,-}$ are the positive and negative components
of the specific strain energies.
$$
- \frac{d\Psi}{dc} \cdot \frac{dc}{dt} = 2 \cdot (1 - c) \cdot G_{0,+} \cdot \frac{dc}{dt}
$$
\cite{miehe2015phasefield}
\cite{chakraborty2016intergranularfracture}
This computes the crack propagation heat source as a Material property
and is intended to be used with the
[CrackPropagationHeatSource](/Kernels/tensor_mechanics/CrackPropagationHeatSource.md)
Kernel to apply this as a term in the heat equation.

!syntax description /Materials/ComputeCrackPropagationHeatEnergy

!syntax parameters /Materials/ComputeCrackPropagationHeatEnergy

!syntax inputs /Materials/ComputeCrackPropagationHeatEnergy

!syntax children /Materials/ComputeCrackPropagationHeatEnergy
