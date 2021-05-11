# SpecificInternalEnergyAux

!syntax description /AuxKernels/SpecificInternalEnergyAux

This AuxKernel computes the specific internal energy $u$ based from the total energy $e$
and the kinetic energy, computed using the fluid momentum $\rho \vec{u}$.

\begin{equation}
u = e - \dfrac{||\rho \vec{u}||_{L^2}}{2 \rho}
\end{equation}

!syntax parameters /AuxKernels/SpecificInternalEnergyAux

!syntax inputs /AuxKernels/SpecificInternalEnergyAux

!syntax children /AuxKernels/SpecificInternalEnergyAux
