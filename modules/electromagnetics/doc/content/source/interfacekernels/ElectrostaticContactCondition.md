# ElectrostaticContactCondition

!syntax description /InterfaceKernels/ElectrostaticContactCondition

## Description

This interface kernel models the conductivity of electric field across a specified
boundary between two dissimilar materials, as described by [!citep](cincotti2007sps).
It accounts for the influence of electrostatic potential differences across the
interface, with an appropriate electrical contact conductance coefficient
being provided either by the user as a constant scalar number or via a combination
of material properties and constants for calculation. The condition being applied is:

\begin{equation}
  \sigma_{el,1} \frac{\partial \phi}{\partial \mathbf{r}} \bigg\rvert_1 \cdot \mathbf{\hat{n}} = \sigma_{el,2} \frac{\partial \phi}{\partial \mathbf{r}} \bigg\rvert_2 \cdot \mathbf{\hat{n}}
\end{equation}

and

\begin{equation}
  \sigma_{el,1} \frac{\partial \phi}{\partial \mathbf{r}} \bigg\rvert_1 \cdot \mathbf{\hat{n}} = -C_E (\phi_1 - \phi_2)
\end{equation}

where

- $\sigma_{el, i}$ is the electrical conductivity of each material along the interface,
- $C_E$ is the electrical contact conductance, and
- $\phi_i$ is the electrostatic potential of the material at the interface.

The temperature- and mechanical-pressure-dependent electrical contact conductance, given by [!citep](babu2001contactresistance), is calculated using:

\begin{equation}
  C_E(T, P) = \alpha_E \sigma_{el,Harm} \bigg( \frac{P}{H_{Harm}} \bigg)^{\beta_E}
\end{equation}

where

- $\alpha_E$ is an experimentally-derived proportional fit parameter (set to be 64, from [!citep](cincotti2007sps)),
- $\sigma_{el,Harm}$ is the harmonic mean of the temperature-dependent electrical conductivities on either side of the boundary,
- $P$ ($=F/S$) is the uniform mechanical pressure applied at the contact surface area (S) between the two materials,
- $H_{Harm}$ is the harmonic mean of the hardness values of each material, and
- $\beta_E$ is an experimentally-derived power fit parameter (set to be 0.35, from [!citep](cincotti2007sps)).

For reference, the harmonic mean calculation for two values, $V_a$ and $V_b$, is given by

\begin{equation}
  V_{Harm} = \frac{2 V_a V_b}{V_a + V_b}
\end{equation}

!alert warning title=Order of variables matters!
Please note that `variable` *must always* refer to the variable of higher potential,
while the `neighbor_var` *must always* refer to the variable of lower potential in
your model. Knowledge of your boundary conditions (where potential is applied or
grounded) and electrical conductivities on either side of the boundary is vital
to making the right choice! Please refer to the electromagnetics module test
examples as well as [!citep](cincotti2007sps) for guidance and usage.

## Example Input File Syntax

!listing contact_conductance_calculated.i block=InterfaceKernels/electrostatic_contact


!syntax parameters /InterfaceKernels/ElectrostaticContactCondition

!syntax inputs /InterfaceKernels/ElectrostaticContactCondition

!syntax children /InterfaceKernels/ElectrostaticContactCondition
