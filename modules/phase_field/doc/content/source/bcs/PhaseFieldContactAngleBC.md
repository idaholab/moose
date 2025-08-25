# PhaseFieldContactAngleBC

!syntax description /BCs/PhaseFieldContactAngleBC

The `PhaseFieldContactAngleBC` kernel implements the contact angle boundary condition based on the phase field framework.

\begin{equation}
    \nabla \phi \cdot \mathbf{n}=\frac{1}{\lambda} \frac{3}{4} \sigma \cos \left(\theta \right)\left(1-\phi^2\right)
\end{equation}

$\lambda$ is the mixing energy density in the interface while $\sigma$ and $\theta$ represent the surface tension and contact angle respectively.

!syntax parameters /BCs/PhaseFieldContactAngleBC

!syntax inputs /BCs/PhaseFieldContactAngleBC

!syntax children /BCs/PhaseFieldContactAngleBC
