# PhaseFieldCoupledDoubleWellPotential

!syntax description /Kernels/PhaseFieldCoupledDoubleWellPotential

The chemical potential($G$) associated with Cahn-Hilliard equation for the advection of the phase field variable ($\phi$) can be written as([!citep](yue2010sharp)):

\begin{equation}

    G= \lambda (- \nabla^2 \phi+ \frac{\left(\phi^2-1\right) \phi}{\epsilon^2})
\end{equation}

where, $\lambda$ is the the mixing energy density and $\epsilon$ is the interface thickness between the phases.

The `PhaseFieldCoupledDoubleWellPotential` implments the second term in the above equation as follows:

\begin{equation}
    prefactor \phi (\phi^2 - 1)
\end{equation}

where $prefactor$ can be specified as per the problem.


!syntax parameters /Kernels/PhaseFieldCoupledDoubleWellPotential

!syntax inputs /Kernels/PhaseFieldCoupledDoubleWellPotential

!syntax children /Kernels/PhaseFieldCoupledDoubleWellPotential

!bibtex bibliography
