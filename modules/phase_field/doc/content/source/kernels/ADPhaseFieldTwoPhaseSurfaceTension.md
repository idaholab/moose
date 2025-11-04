# ADPhaseFieldTwoPhaseSurfaceTension

!syntax description /Kernels/ADPhaseFieldTwoPhaseSurfaceTension

To account for the surface tension force in a capillary flows, a body force term has to be included on the right hand side of the Navier-Stokes equations. The `ADPhaseFieldTwoPhaseSurfaceTension` kernel performs this coupling by using the force formulation as specified in ([!citep](yue2010sharp)).

\begin{equation}
    \begin{gathered}
        &\mathbf{F}_{\mathrm{st}} =  \frac{\lambda}{\epsilon^2} \psi \nabla \phi$ \\
        $\psi=-\epsilon^2 \nabla^2 \phi+\left(\phi^2-1\right) \phi
    \end{gathered}
\end{equation}

The details of the phase field parameters ($\gamma$,$\lambda$ and $\epsilon$ ) can be found in the reference. 

!syntax parameters /Kernels/ADPhaseFieldTwoPhaseSurfaceTension

!syntax inputs /Kernels/ADPhaseFieldTwoPhaseSurfaceTension

!syntax children /Kernels/ADPhaseFieldTwoPhaseSurfaceTension

!bibtex bibliography
