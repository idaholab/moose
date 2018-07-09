# PorousFlowConstantThermalExpansionCoefficient

!syntax description /Materials/PorousFlowConstantThermalExpansionCoefficient

This Material computes
\begin{equation}
\label{constthermexpdef}
A = (\alpha_{B} - \phi)\alpha_{T} + \phi\alpha_{f} \ ,
\end{equation}
where $\alpha_{B}$ is the Biot Modulus, $\phi$ is the porosity, $\alpha_{T}$ is the drained volumetric thermal expansion coefficient, and $\alpha_{f}$ is the fluid thermal volumetric expansion coefficient.

This Material is designed to work with the [PorousFlowFullySaturatedMassTimeDerivative](PorousFlowFullySaturatedMassTimeDerivative.md) Kernel.

!alert note
This quantity is computed during the initial stage of the simulation and is kept fixed thereafter.

!syntax parameters /Materials/PorousFlowConstantThermalExpansionCoefficient

!syntax inputs /Materials/PorousFlowConstantThermalExpansionCoefficient

!syntax children /Materials/PorousFlowConstantThermalExpansionCoefficient
