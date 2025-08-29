# ObtainAvgContactAngle

!syntax description /Postprocessors/ObtainAvgContactAngle

The `ObtainAvgContactAngle` kernel postprocesses the phase field variable to calculate the contact angle for verification. The kernel iterates through the boundary cells where $-0.5<\phi<0.5$

\begin{equation}
    \theta = \frac{\cosh{\frac{\nabla \phi \cdot \mathbf{n}}{\left| \nabla \phi \right|}} ~ 180}{\pi}
\end{equation}


!syntax parameters /Postprocessors/ObtainAvgContactAngle

!syntax inputs /Postprocessors/ObtainAvgContactAngle

!syntax children /Postprocessors/ObtainAvgContactAngle

