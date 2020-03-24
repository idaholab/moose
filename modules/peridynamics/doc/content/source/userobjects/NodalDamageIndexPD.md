# Nodal Damage Index UserObject

## Description

The `NodalDamageIndexPD` UserObject computes the value for a AuxVariable by calculating the damage index at each material point as the ratio of total volume of intact bonds to total volume all bonds associated with a material point.

\begin{equation}
  \phi \left( \mathbf{X}^{\prime} - \mathbf{X}, t \right) = 1 -\frac{\int_{\mathcal{H}_{\mathbf{X}}} \mu \left( \mathbf{X}^{\prime} - \mathbf{X}, t \right) dV^{\prime}}{\int_{\mathcal{H}_{\mathbf{X}}} 1 dV^{\prime}}
\end{equation}
where $\mu$ is the bond status parameter whose value is computed using AuxKernel `BondStatusPD`.

!syntax parameters /UserObjects/NodalDamageIndexPD

!syntax inputs /UserObjects/NodalDamageIndexPD

!syntax children /UserObjects/NodalDamageIndexPD
