# ContactPressureAux

## Description

The `ContactPressureAux` outputs the normal pressure incurred due to two bodies coming into contact. It is computed via:
\begin{equation*}
  p_N = \frac{\boldsymbol{f}\cdot \boldsymbol{n}}{A},
\end{equation*}
where $p_N \in \mathbb{R}$ denotes the contact pressure, $\boldsymbol{f} \in \mathbb{R}^{d}$ represents the contact force in $d$ dimensions, and $A \in \mathbb{R}$ is the nodal area (see [Nodal Area](userobjects/NodalArea.md)).  This object is an [AuxKernel](AuxKernels/index.md), and is used only for the purpose of output. Note that the [Contact](Contact/index.md) action sets this object up automatically, so it is typically not necessary to include this in an input file.


!syntax parameters /AuxKernels/ContactPressureAux

!syntax inputs /AuxKernels/ContactPressureAux

!syntax children /AuxKernels/ContactPressureAux
