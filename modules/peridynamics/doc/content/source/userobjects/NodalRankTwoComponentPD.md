# Nodal Rank Two Component UserObject

## Description

The `NodalRankTwoComponentPD` UserObject is used to compute the values of rank two tensor components at each material point for correspondence material model. The component variable should be defined as aux variable, but its value is computed using UserObject rather than AuxKernel.

In self-stabilized correspondence material model, a rank two tensor (e.g., strain and stress) at a material point is the weighted average of bond-associated corresponding rank two tensors connected at that material point.

\begin{equation}
  \mathbf{A}_{\mathbf{X}} =\frac{\sum_{n=1}^{NP} w \left\langle \boldsymbol{\xi}_n \right\rangle \mathbf{A}_{\boldsymbol{\xi}_n}}{\sum_{n=1}^{NP}w \left \langle \boldsymbol{\xi}_n \right \rangle}
\end{equation}
where $\mathbf{A}_{\boldsymbol{\xi}_n}$ is the bond-associated rank two tensor, $w \left\langle \boldsymbol{\xi}_n \right\rangle$ is the weight for each bond-associated rank two tensor.

!syntax parameters /UserObjects/NodalRankTwoComponentPD

!syntax inputs /UserObjects/NodalRankTwoComponentPD

!syntax children /UserObjects/NodalRankTwoComponentPD
