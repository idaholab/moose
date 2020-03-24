# Nodal Functions L2 Norm Postprocessor

## Description

The `NodalFunctionsL2NormPD` Postprocessor calculates the L2 norm of a set of functions using the discrete summation formulation.

\begin{equation}
  \text{L2} = \int_{\Omega} {\left| \mathbf{f} \right|} d{\Omega} = \sum V_{\mathbf{X}} \cdot {\left| \bar{\mathbf{f}}_{\mathbf{X}} \right|}
\end{equation}
where $V_{\mathbf{X}}$ is the area or volume of material point $\mathbf{X}$, and $\bar{\mathbf{f}}_{\mathbf{X}}$ is the evaluation of functions $\mathbf{f}$ at material point $\mathbf{X}$.

!syntax parameters /Postprocessors/NodalFunctionsL2NormPD

!syntax inputs /Postprocessors/NodalFunctionsL2NormPD

!syntax children /Postprocessors/NodalFunctionsL2NormPD
