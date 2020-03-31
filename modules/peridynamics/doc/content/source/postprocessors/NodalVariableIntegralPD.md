# Nodal Variable Integral Postprocessor

## Description

The `NodalVariableIntegralPD` Postprocessor calculates the volume (3D) or area (2D) integral of nodal variables using the discrete summation formulation.

\begin{equation}
  \text{val} = \int_{\Omega} u d{\Omega} = \sum V_{\mathbf{X}} \cdot \bar{u}_{\mathbf{X}}
\end{equation}
where $V_{\mathbf{X}}$ is the area or volume of material point $\mathbf{X}$, and $\bar{u}_{\mathbf{X}}$ is the evaluation of variable $u$ at material point $\mathbf{X}$.

!syntax parameters /Postprocessors/NodalVariableIntegralPD

!syntax inputs /Postprocessors/NodalVariableIntegralPD

!syntax children /Postprocessors/NodalVariableIntegralPD
