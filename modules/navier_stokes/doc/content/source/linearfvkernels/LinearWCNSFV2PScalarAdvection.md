# LinearWCNSFV2PScalarAdvection

This kernel adds the advection of the dispersed phase fraction of a two-phase mixture to the matrix
and right hand side of the phase transport equation, for the finite volume SIMPLE segregated solver
[SIMPLE.md]. It extends [LinearFVScalarAdvection.md], whose mixture flux, interpolation methods and
conservative form it shares, with the drift of the dispersed phase relative to the mixture. It is
added by [WCNSLinearFVTwoPhaseMixturePhysics.md] for the phase fraction.

## The drift flux, and how it is assembled

The advecting velocity is the dispersed phase velocity
$\vec{u}_m + \vec{u}_{Md}$ rather than the mixture velocity alone, and the face flux acquires a
drift contribution. The mixture flux and the drift flux are interpolated separately, each against
its own scheme and each upwinding in its own direction. The drift uses the scheme named by
[!param](/LinearFVKernels/LinearWCNSFV2PScalarAdvection/drift_advected_interp_method_name), which
defaults to the mixture flux's
[!param](/LinearFVKernels/LinearWCNSFV2PScalarAdvection/advected_interp_method_name):

\begin{equation}
\int\limits_{V_C}\nabla\cdot\left(\vec{u}\,C_i\right)dV \approx
  \sum\limits_f \left[ F^m_f\, C_{if}(F^m_f) + F^d_f\, C_{if}(F^d_f) \right] ,
\qquad
F^m_f + F^d_f = F_f
\end{equation}

Each part is separately in donor cell form, so for an upwind scheme the sum is an M-matrix: its
off-diagonal entries are non-positive and its diagonal dominates them, which guarantees that the
solve introduces no new extrema and keeps the phase fraction bounded. On a
face where the two fluxes share a sign they select the same donor cell and the assembly is
identical to interpolating their sum once. They differ only where the drift
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

The price is that the scheme is more diffusive, each part upwinding independently.

The drift contributes on every face the variable is defined on from both sides, and on a boundary
face only where that boundary is listed in
[!param](/LinearFVKernels/LinearWCNSFV2PScalarAdvection/slip_boundaries). This includes boundaries
internal to the mesh, such as the interface with a block the variable is not defined on. The dispersed phase
cannot cross an impermeable wall, and the mixture flux returned by the Rhie-Chow object is already
zero there, so adding a gravity driven drift would advect the phase straight through it.

When [!param](/LinearFVKernels/LinearWCNSFV2PScalarAdvection/density) is supplied, the drift flux
is multiplied by the same density as the mixture flux, so that the density multiplies the whole
phase velocity.

!syntax parameters /LinearFVKernels/LinearWCNSFV2PScalarAdvection

!syntax inputs /LinearFVKernels/LinearWCNSFV2PScalarAdvection

!syntax children /LinearFVKernels/LinearWCNSFV2PScalarAdvection
