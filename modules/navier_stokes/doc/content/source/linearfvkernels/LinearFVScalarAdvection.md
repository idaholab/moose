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

## The drift flux, and how it is assembled

When the slip velocity parameters [!param](/LinearFVKernels/LinearFVScalarAdvection/u_slip) and
its companions are supplied, the advecting velocity is the dispersed phase velocity
$\vec{u}_m + \vec{u}_{Md}$ rather than the mixture velocity alone, and the face flux acquires a
drift contribution. The mixture flux and the drift flux are interpolated separately, each against
the scheme named by
[!param](/LinearFVKernels/LinearFVScalarAdvection/advected_interp_method_name) and each upwinding
in its own direction:

\begin{equation}
\int\limits_{V_C}\nabla\cdot\left(\vec{u}\,C_i\right)dV \approx
  \sum\limits_f \left[ F^m_f\, C_{if}(F^m_f) + F^d_f\, C_{if}(F^d_f) \right] ,
\qquad
F^m_f + F^d_f = F_f
\end{equation}

Each part is separately in donor cell form, so for an upwind scheme the sum is an M-matrix. On a
face where the two fluxes share a sign they select the same donor cell and the assembly is
identical, to the last bit, to interpolating their sum once. They differ only where the drift
opposes the mixture flux, and there the diagonal receives

\begin{equation}
\max\left(F^m_f, 0\right) + \max\left(F^d_f, 0\right) \;\ge\;
\max\left(F^m_f + F^d_f,\, 0\right)
\end{equation}

so the split assembly is never less diagonally dominant than a single interpolation of the sum, and
is strictly more so wherever the phases move against each other. That is what holds the solve
together at high dispersed phase fraction: with the sum interpolated once, a vertical bubbly flow at
a dispersed phase fraction of $0.25$ loses control of the phase fraction and the solve fails part
way through the transient.

Splitting also makes the drift a term of its own, which is the prerequisite for limiting it
separately, in the manner of the MULES treatment used by OpenFOAM's `driftFluxFoam`. The price is
that the scheme is more diffusive, each part upwinding independently.

The slip contributes on internal faces always, and on a boundary face only where that boundary is
listed in [!param](/LinearFVKernels/LinearFVScalarAdvection/slip_boundaries). The dispersed phase
cannot cross an impermeable wall, and the mixture flux returned by the Rhie--Chow object is already
zero there, so adding a gravity driven slip would advect the phase straight through it.

## The conservative form

When [!param](/LinearFVKernels/LinearFVScalarAdvection/density) is supplied the volumetric fluxes
are turned into mass fluxes, so that $\nabla\cdot\left(\rho\,\vec{u}\,C_i\right)$ is assembled
rather than $\nabla\cdot\left(\vec{u}\,C_i\right)$. The density is applied after the slip, so
that it multiplies the whole phase velocity, and before the interpolations, so that the upwind
directions are unaffected by it. The matching `factor` must be set on the time derivative kernel,
and the same density must reach every other term of the equation, for the pair to be consistent.

!syntax parameters /LinearFVKernels/LinearFVScalarAdvection

!syntax inputs /LinearFVKernels/LinearFVScalarAdvection

!syntax children /LinearFVKernels/LinearFVScalarAdvection
