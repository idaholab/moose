# LinearFVEnergyAdvection

This kernel adds the contributions of the energy advection term to the matrix and right hand side of the energy equation system for the finite volume SIMPLE segregated solver [SIMPLE.md].

This kernel currently supports the advection of specific enthalpy $h$ or temperature $T$. Important consideration: Temperature advection is only supported for constant specific heat, where $h$ can be defined as $h=c_p T$. For variable $c_p$, the user should use the enthalpy formulation. Parameter [!param](/LinearFVKernels/LinearFVEnergyAdvection/advected_quantity) lets the user select "enthalpy" or "temperature".

This term is described by $\nabla \cdot \left(\rho\vec{u} h \right)$ for enthalpy or $\nabla \cdot \left(\rho\vec{u} c_p T \right)$ for constant specific heat. This term is present in the energy equation conservation for an incompressible/weakly-compressible formulation.

For FV, the integral of the advection term over a cell can be expressed as:

\begin{equation}
\int\limits_{V_C} \nabla \cdot \left(\rho\vec{u} h \right) dV \approx \sum\limits_f (\rho \vec{u}\cdot \vec{n})_{RC} h_f |S_f| \,
\end{equation}

where $h_f$ is a face enthalpy. The enthalpy acts as the advected quantity and an interpolation scheme (e.g. upwind) can be used to compute the face value. This kernel adds the face contribution for each face $f$ to the right hand side and matrix.

The face mass flux $(\rho \vec{u}\cdot \vec{n})_{RC}$ is provided by the [RhieChowMassFlux.md] object which uses pressure
gradients and the discrete momentum equation to compute face velocities and mass fluxes.
For more information on the expression that is used, see [SIMPLE.md].

!syntax parameters /LinearFVKernels/LinearFVEnergyAdvection

!syntax inputs /LinearFVKernels/LinearFVEnergyAdvection

!syntax children /LinearFVKernels/LinearFVEnergyAdvection
