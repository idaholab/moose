# Stretch-based Failure Criterion AuxKernel

## Description

The `StretchBasedFailureCriterionPD` AuxKernel is to determine the status $\mu$ of each individual bond using critical stretch failure criterion. For critical stretch failure criterion, if the bond's mechanical stretch $s$ less than its critical value $s_{0}$ and intact in previous simulation step, the bond is considered as intact and a value of 1 is returned, otherwise, the bond is broken and a value of 0 is returned.

### Critical stretch failure criterion

\begin{equation}
  \mu \left( \mathbf{X}^{\prime} - \mathbf{X}, t \right) = \left\{\begin{matrix}
  1 & s < s_{0} \\
  0 & s > s_{0}
  \end{matrix}\right.
\end{equation}

The bond critical stretch value can be calculated for the bond-based models from the critical energy release rate $G_c$ as

\begin{equation}
  s_{0} = \left\{\begin{matrix}
  \sqrt{\frac{\pi G_{c}}{3K \delta}} & \text{two dimensional} \\
  \sqrt{\frac{5 G_{c}}{9K \delta}} & \text{three dimensional}
  \end{matrix}\right.
\end{equation}
where $K$ is the bulk modulus and $\delta$ is the horizon size.

Derivation of above relationships can be found in Ref. [!citep](Silling2005meshfree).


!syntax parameters /AuxKernels/StretchBasedFailureCriterionPD

!syntax inputs /AuxKernels/StretchBasedFailureCriterionPD

!syntax children /AuxKernels/StretchBasedFailureCriterionPD
