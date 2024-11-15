# LinearFVEnergyAdvection

This kernel adds the contributions of the energy advection term to the matrix and right hand side of the energy equation system for the finite volume SIMPLE segregated solver [SIMPLE.md].

This term is described by $\nabla \cdot \left(\rho\vec{u} c_p T \right)$ present in the energy equation conservation for an incompressible/weakly-compressible formulation. Currently, the kernel only supports +constant specific heat values+ for $c_p$ and solves for a temperature variable. The specific heat value needs to be prescribed, otherwise it will default to its default value of $c_p=1$.

For FV, the integral of the advection term over a cell can be expressed as:

\begin{equation}
\int\limits_{V_C} \nabla \cdot \left(\rho\vec{u} c_p T \right) dV \approx \sum\limits_f (\rho \vec{u}\cdot \vec{n})_{RC} c_p T_f |S_f| \,
\end{equation}

where $T_f$ is a face temperature. The temperature acts as the advected quantity and an interpolation scheme (e.g. upwind) can be used to compute the face value. This kernel adds the face contribution for each face $f$ to the right hand side and matrix.

The face mass flux $(\rho \vec{u}\cdot \vec{n})_{RC}$ is provided by the [RhieChowMassFlux.md] object which uses pressure
gradients and the discrete momentum equation to compute face velocities and mass fluxes.
For more information on the expression that is used, see [SIMPLE.md].

!syntax parameters /LinearFVKernels/LinearFVEnergyAdvection

!syntax inputs /LinearFVKernels/LinearFVEnergyAdvection

!syntax children /LinearFVKernels/LinearFVEnergyAdvection
