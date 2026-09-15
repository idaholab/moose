# ObtainAvgContactAngle

!syntax description /Postprocessors/ObtainAvgContactAngle

The `ObtainAvgContactAngle` postprocessor postprocesses the phase field variable to calculate the contact angle on a boundary for verification. The pointwise angle follows from the interface normal $\nabla \phi / \left| \nabla \phi \right|$ and is averaged over the boundary $\Gamma$ with the weight $\left| \nabla \phi \right| \left( 1 - \phi^2 \right)$. The factor $\left| \nabla \phi \right|$ removes the pointwise division and $\left( 1 - \phi^2 \right)$ confines the average to the interface, where the normal is defined:

\begin{equation}
    \theta = \frac{180}{\pi} \arccos \left( \frac{\int_{\Gamma} \left( 1 - \phi^2 \right) \, \nabla \phi \cdot \mathbf{n} \, dA}{\int_{\Gamma} \left( 1 - \phi^2 \right) \left| \nabla \phi \right| \, dA} \right)
\end{equation}


!syntax parameters /Postprocessors/ObtainAvgContactAngle

!syntax inputs /Postprocessors/ObtainAvgContactAngle

!syntax children /Postprocessors/ObtainAvgContactAngle

