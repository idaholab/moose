# PorousFlowPorosityLinear

This Material computes porosity (at the nodes or quadpoints, depending on the `at_nodes` flag) according to

\begin{equation}
\label{eq:poro_evolve_linear}
\phi = \phi_{\mathrm{ref}} + A(P_{f} - P_{f\ \mathrm{ref}}) + B(T - T_{\mathrm{ref}}) + C(\epsilon^{\mathrm{total}}_{ii} - \epsilon^{\mathrm{total}}_{ii\ \mathrm{ref}}) \ .
\end{equation}

Here:

- $\phi_{\mathrm{ref}}$ is the reference porosity, which is a constant-monomial variable or a real number
- $A$, $B$ and $C$ are real-valued coefficients
- $P_{f}$ is the [effective fluid pressure](PorousFlowEffectiveFluidPressure.md)
- $P_{f\ \mathrm{ref}}$ is the reference effective fluid pressure, which is a coupled variable (usually linear-Lagrange) or a real number
- $T$ is the [temperature](PorousFlowTemperature.md)
- $T_{\mathrm{ref}}$ is the reference temperature, which is a coupled variable (usually linear-Lagrange) or a real number
- $\epsilon^{\mathrm{total}}_{ii}$ is the [total volumetric strain](PorousFlowVolumetricStrain.md)
- $\epsilon^{\mathrm{total}}_{ii\ \mathrm{ref}}$ is the reference total volumetric strain, which is a constant monomial coupled variable or a real number.

In addition, it is possible to place a lower bound on porosity: $\phi \geq \phi_{\mathrm{min}}$.  This can be physically useful, but can also be numerically important because during Newton-Raphson iterations, the linear relationship can sometimes yield small, or even negative, values of $\phi$.

Descriptions of other porosity classes can be found in the [porosity documentation](/porous_flow/porosity.md)

!syntax parameters /Materials/PorousFlowPorosityLinear

!syntax inputs /Materials/PorousFlowPorosityLinear

!syntax children /Materials/PorousFlowPorosityLinear
