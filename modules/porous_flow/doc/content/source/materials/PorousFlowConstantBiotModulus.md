# PorousFlowConstantBiotModulus

!syntax description /Materials/PorousFlowConstantBiotModulus

The Biot Modulus is defined to be
\begin{equation}
\label{biotmoddef}
\frac{1}{M} = \frac{\phi}{K_{f}} + \frac{(1 - \alpha_{B})(\alpha_{B} -
  \phi)}{K} \ ,
\end{equation}
where $\phi$ is the porosity, $K_{f}$ is the fluid bulk modulus, $\alpha_{B}$ is the Biot coefficient and $K$ is the drained bulk modulus of the porous skeleton.

This Material is designed to work with the [PorousFlowFullySaturatedMassTimeDerivative](PorousFlowFullySaturatedMassTimeDerivative.md) Kernel.

!alert note
This quantity is computed during the initial stage of the simulation and is kept fixed thereafter.

!syntax parameters /Materials/PorousFlowConstantBiotModulus

!syntax inputs /Materials/PorousFlowConstantBiotModulus

!syntax children /Materials/PorousFlowConstantBiotModulus
