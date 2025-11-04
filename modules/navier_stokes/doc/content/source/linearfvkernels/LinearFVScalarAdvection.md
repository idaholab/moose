# LinearFVScalarAdvection

This kernel adds the contributions of the scalar advection term to the matrix and right hand side of the scalar equation system for the finite volume SIMPLE segregated solver [SIMPLE.md].

This term is described by $\nabla \cdot \left(\vec{u} C_i \right)$ present in the scalar equation conservation for an incompressible/weakly-compressible formulation.

For FV, the integral of the advection term of scalar $C_i$ over a cell can be expressed as:

\begin{equation}
\int\limits_{V_C} \nabla \cdot \left(\vec{u} C_i \right) dV \approx \sum\limits_f ( \vec{u}\cdot \vec{n})_{RC} C_if |S_f| \,
\end{equation}

where $C_{if}$ is the face value of the scalar concentration. An interpolation scheme (e.g. upwind) can be used to compute the face value. This kernel adds the face contribution for each face $f$ to the right hand side and matrix.

The volumetric face flux $(\vec{u}\cdot \vec{n})_{RC}$ is provided by the [RhieChowMassFlux.md] object which uses pressure
gradients and the discrete momentum equation to compute face velocities and mass fluxes.
For more information on the expression that is used, see [SIMPLE.md].

!syntax parameters /LinearFVKernels/LinearFVScalarAdvection

!syntax inputs /LinearFVKernels/LinearFVScalarAdvection

!syntax children /LinearFVKernels/LinearFVScalarAdvection
