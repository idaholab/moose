# Bond Mechanical Stretch AuxKernel

## Description

AuxKernel `MechanicalStretchPD` is used to output the mechanical stretch for each bond from the material class.

A bond total stretch is defined as the change in bond length divided by initial bond length

\begin{equation}
  s = \frac{\left(\left|\mathbf{x}^{\prime}\left(\mathbf{X}^{\prime},t\right) - \mathbf{x}\left(\mathbf{X},t\right)\right| - \left| \mathbf{X}^{\prime} - \mathbf{X} \right|\right)}{\left| \mathbf{X}^{\prime} - \mathbf{X} \right|}
\end{equation}
where $\mathbf{X}$ and $\mathbf{X}^{\prime}$ are the reference positions of the two end material points connected by the bond, and $\mathbf{x}$ and $\mathbf{x}^{\prime}$ are the corresponding positions in current configuration.

Mechanical stretch is the subtraction of eigen- stretches from total stretch.

!syntax parameters /AuxKernels/MechanicalStretchPD

!syntax inputs /AuxKernels/MechanicalStretchPD

!syntax children /AuxKernels/MechanicalStretchPD
