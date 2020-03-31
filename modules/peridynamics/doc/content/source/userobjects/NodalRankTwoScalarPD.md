# Nodal Rank Two Scalar UserObject

## Description

The `NodalRankTwoScalarPD` UserObject is used to compute the values of equivalent scalar quantities of a rank two tensor at each material point for correspondence material model. The scalar variable should be defined as aux variable, but its value is computed using UserObject rather than AuxKernel.

In self-stabilized correspondence material model, a equivalent scalar quantity of rank two tensor (e.g., von Mises stress) at a material point is the weighted average of bond-associated corresponding equivalent quantities connected at that material point.

\begin{equation}
  \alpha_{\mathbf{X}} =\frac{\sum_{n=1}^{NP} w \left\langle \boldsymbol{\xi}_n \right\rangle \alpha_{\boldsymbol{\xi}_n}}{\sum_{n=1}^{NP}w \left \langle \boldsymbol{\xi}_n \right \rangle}
\end{equation}
where $\alpha_{\boldsymbol{\xi}_n}$ is the bond-associated scalar quantity of rank two tensor, $w \left\langle \boldsymbol{\xi}_n \right\rangle$ is the weight for each bond-associated scalar quantity of rank two tensor. For current implementation, the volume fraction is used as the weight.

!syntax parameters /UserObjects/NodalRankTwoScalarPD

!syntax inputs /UserObjects/NodalRankTwoScalarPD

!syntax children /UserObjects/NodalRankTwoScalarPD
