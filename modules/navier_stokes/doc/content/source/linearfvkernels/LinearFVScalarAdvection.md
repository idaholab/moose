# LinearFVScalarAdvection

This kernel adds the contributions of the scalar advection term to the matrix and right hand side of the scalar equation system for the finite volume SIMPLE segregated solver [SIMPLE.md].

This term is described by $\nabla \cdot \left(\vec{u} C_i \right)$ present in the scalar equation conservation for an incompressible/weakly-compressible formulation.

For FV, the integral of the advection term of scalar $C_i$ over a cell can be expressed as:

\begin{equation}
\int\limits_{V_C} \nabla \cdot \left(\vec{u} C_i \right) dV \approx \sum\limits_f ( \vec{u}\cdot \vec{n})_{RC} C_if |S_f| \,
\end{equation}

where $C_{if}$ is the face value of the scalar concentration. An interpolation scheme (e.g. upwind) can be used to compute the face value. This kernel adds the face contribution for each face $f$ to the right hand side and matrix.

## Selecting the interpolation method

The [!param](/LinearFVKernels/LinearFVScalarAdvection/advected_interp_method_name) parameter is
the name of an interpolation method in the `[FVInterpolationMethods]` block. For example, to use
Van Leer interpolation, add an [FVAdvectedVanLeerWeightBased.md] method and set
[!param](/LinearFVKernels/LinearFVScalarAdvection/advected_interp_method_name) to that method name:

!listing modules/navier_stokes/test/tests/finite_volume/ins/channel-flow/linear-segregated/2d-scalar/channel.i block=FVInterpolationMethods

!listing modules/navier_stokes/test/tests/finite_volume/ins/channel-flow/linear-segregated/2d-scalar/channel.i block=LinearFVKernels/s1_advection

When using [WCNSLinearFVScalarTransportPhysics.md], the
[!param](/Physics/NavierStokes/ScalarTransportSegregated/WCNSLinearFVScalarTransportPhysics/passive_scalar_advection_interpolation)
shortcut can be set directly, for example `passive_scalar_advection_interpolation = min_mod`. No
`[FVInterpolationMethods]` block is needed for the Physics shortcut.

The volumetric face flux $(\vec{u}\cdot \vec{n})_{RC}$ is provided by the [RhieChowMassFlux.md] object which uses pressure
gradients and the discrete momentum equation to compute face velocities and mass fluxes.
For more information on the expression that is used, see [SIMPLE.md].

!syntax parameters /LinearFVKernels/LinearFVScalarAdvection

!syntax inputs /LinearFVKernels/LinearFVScalarAdvection

!syntax children /LinearFVKernels/LinearFVScalarAdvection
