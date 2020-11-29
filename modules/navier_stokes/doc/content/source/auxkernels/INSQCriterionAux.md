# INSQCriterionAux

This calculates a scalar variable calculated as a function of the shear rate
properties of a fluid flow called $Q$ which is useful for visualizing complicated
flow patterns which tend to occur when running turbulent fluid flow simulations.
The quantity is calculated as:

\begin{equation}
Q = \frac{1}{2} (|| \bar{\bar{\Omega}} || - ||\bar{\bar{S}}||)
\end{equation}

With $\bar{\bar{\Omega}}$ and $\bar{\bar{S}}$ respectively being the antisymmetric
and symmetric strain rate tensors.
By taking the contour of $Q$ in a piece of visualization software where $Q=0$,
boundaries of vortices in the flow should be revealed. For more information,
see the paper [!cite](jeong1995).

!syntax description /AuxKernels/INSQCriterionAux

!syntax parameters /AuxKernels/INSQCriterionAux

!syntax inputs /AuxKernels/INSQCriterionAux

!syntax children /AuxKernels/INSQCriterionAux
