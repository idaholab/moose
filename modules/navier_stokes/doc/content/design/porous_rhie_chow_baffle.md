## Relevant discrete forms

Notation (cell P, face f):

- $P$: owner cell for face $f$
- $N$: neighbor cell across face $f$
- $S_f$: outward face area vector (points from $P$ to $N$)
- $V_P$: cell volume
- $\rho$: density
- $\epsilon$: porosity
- $u$: velocity unknown (superficial form unless otherwise stated)
- $\phi_f$: face mass flux
- $\alpha_P, \alpha_N$: face interpolation coefficients (from the selected advection scheme)

Continuity (finite volume):

\begin{equation}
\sum_f \phi_f = 0
\end{equation}

Face mass flux (Rhie-Chow form, schematic):

\begin{equation}
\phi_f = HbyA_f - \left(A^{-1}_f S_f\right) \cdot (\nabla p)_f
\end{equation}

Momentum advection (component $i$, finite volume):

\begin{equation}
\sum_f \left(\phi_f \, u_{i,f}\right) = 0
\end{equation}

Face value for advection (two equivalent porous placements):

Porosity outside divergence:

\begin{equation}
u_{i,f} = \alpha_P u_{i,P} + \alpha_N u_{i,N}
\end{equation}

Advection term scaled by $\epsilon_f$.

Porosity inside divergence:

\begin{equation}
u_{i,f} = \alpha_P \left(\frac{u_{i,P}}{\epsilon_P}\right) + \alpha_N \left(\frac{u_{i,N}}{\epsilon_N}\right)
\end{equation}

No additional $\epsilon_f$ scaling.

Pressure gradient in momentum (cell-centered Gauss):

\begin{equation}
(\nabla p)_P = \frac{1}{V_P} \sum_f \left(p_f S_f\right)
\end{equation}

Pressure equation (schematic FV Poisson):

\begin{equation}
\sum_f \left(A^{-1}_f S_f\right) \cdot (\nabla p)_f = \sum_f HbyA_f
\end{equation}

Baffle jump enforcement (face-based):

\begin{equation}
J = \frac{1}{2}\left(\rho_{P} u^2_{n,P} - \rho_{N} u^2_{n,N}\right)
\end{equation}

\begin{equation}
u_{n,P} = \frac{U_n}{\epsilon_{P}}, \quad
u_{n,N} = \frac{U_n}{\epsilon_{N}}, \quad
U_n = \frac{\phi_f}{\rho_f}
\end{equation}

Jump-aware face pressure interpolation:

\begin{equation}
p_f = \alpha_P p_P + \alpha_N \left(p_N + J_{\text{side}}\right)
\end{equation}

\begin{equation}
J_{\text{side}} =
\begin{cases}
J, & \text{N side} \\
-J, & \text{P side}
\end{cases}
\end{equation}

Optional flux-based velocity reconstruction (least-squares, schematic):

\begin{equation}
M = \sum_f \frac{S_f \otimes S_f}{|S_f|}, \quad
b = \sum_f \left(\frac{F_f - \text{corr}_f}{|S_f|}\right) S_f, \quad
u_P = M^{-1} b
\end{equation}

with $\text{corr}_f$ built from the previous-corrector gradients to avoid re-introducing oscillations.

## Major Design Choices

### 1) Porous-specific Rhie-Chow UserObject

A dedicated porous Rhie-Chow mass-flux provider is introduced as a specialization that augments the base Rhie-Chow object with porous and baffle behavior. The Rhie-Chow interpolation is the central point where pressure and momentum communicate. For porous flow, it is the most natural location to:

- Compute face mass fluxes consistent with porosity.
- Store and update pressure jump data across porous interfaces.
- Provide corrected pressure gradients back to pressure kernels.

The pressure jump should be tied to mass-flux-derived normal velocity so
the discontinuity is consistent with the actual flux that enforces
continuity. This reduces mismatch between pressure correction and
momentum advection. The jump update is also under-relaxed, reflecting
the stiff coupling between pressure and flux at sharp porosity changes.

### 2) Pressure equation with a baffle-jump source

A specialized diffusion kernel adds a jump contribution on internal
faces tagged as baffles The Bernoulli jump is a pressure discontinuity
and must be represented in the pressure solve, not just in
post-processing. Injecting the jump into the Poisson equation enforces
it in the global pressure field.
- Makes the discontinuity explicit and stable.
- Ensures the correction step enforces the right total pressure drop at the baffle.

### 3) Momentum pressure term uses a Rhie-Chow-Consistent gradient

A dedicated momentum kernel takes the pressure gradient from the Rhie-Chow provider, optionally using the baffle-corrected gradient.
The momentum equation must see the same discontinuity as the pressure equation to avoid cancellation errors or oscillations.
The result is a closed, self-consistent pressure-velocity coupling across porous interfaces.

### 4) Porosity treatment in advection

The momentum advection kernel includes a switch that determines whether porosity is applied outside the divergence (i.e., scale the flux) or absorbed into the advected interpolation (i.e., scale by \(1/\varepsilon\)). This flexibility is essential when transitioning between discontinuous baffle-type porosity and smoothly varying porosity fields.

### 5) Optional Flux-Based Velocity Reconstruction

What changed: An optional reconstruction step recovers cell velocities from corrected face fluxes.

Why: Classic pressure correction can re-introduce checkerboarding in cell velocities even when face fluxes are smooth. Reconstructing from fluxes suppresses this back-projection of oscillations.

Physics/Numerics rationale: Using corrected face fluxes as the primary source of truth ensures the reconstructed velocity is mass-consistent and reduces pressure-velocity decoupling.

### 6) Pressure gradient limiting at selected sidesets

The porous Rhie-Chow object supports limiting the gradient reconstruction on specific sidesets (e.g., to a one-term expansion).
Limiting reconstruction locally preserves global accuracy while stabilizing baffle-adjacent cells.

### 7) Robust Face Coupling Across Porosity Jumps

The pressure-coupling coefficients can use harmonic
interpolation when porosity is active.
Discontinuous coefficients are better handled by harmonic averaging
to prevent overly diffusive or unstable fluxes.
This improves stability and accuracy when sharp porosity changes dominate the pressure correction.

## Consistency and coupling strategy

The design makes a deliberate consistency effort for the following steps:

1) Pressure solve includes the jump,
2) Rhie-Chow flux uses that jump,
3) Momentum pressure term uses the corrected gradient.

This ensures the discontinuity is not double-counted or accidentally
canceled and that the flux field remains compatible with the pressure
field.
