# LinearFVFunctorRadiativeBC

## Description

`LinearFVFunctorRadiativeBC` applies a radiative heat flux boundary condition to a
linear finite volume (LinearFV) variable. It is the LinearFV counterpart of
[FVFunctorRadiativeBC.md].

The radiative outward heat flux at a boundary is:

$$
q_\text{out} = \sigma \varepsilon \left( T^4 - T_\infty^4 \right)
$$

where $\sigma$ is the Stefan-Boltzmann constant, $\varepsilon$ is the surface emissivity
(supplied as a functor), and $T_\infty$ is the far-field radiation temperature.

## Second-Order Linearization

Because the LinearFV framework assembles an explicit linear system, the nonlinear $T^4$
term must be **Newton-linearized**. For second-order spatial accuracy, the linearization
is performed around the **extrapolated boundary face temperature** $T_{b,\text{old}}$
(rather than the cell-center value):

$$
T_{b,\text{old}} = T_{P,\text{old}} + \nabla T_\text{old} \cdot \mathbf{d}_{cf}
$$

where $\mathbf{d}_{cf}$ is the vector from the cell center to the face centroid.
Linearizing $q_\text{out}$ around $T_{b,\text{old}}$ yields:

$$
k \frac{\partial T}{\partial n} +
\underbrace{4 \sigma \varepsilon T_{b,\text{old}}^3}_{\beta} T_b
= \underbrace{\sigma \varepsilon \left( 3 T_{b,\text{old}}^4 + T_\infty^4 \right)}_{\gamma}
$$

This is a Robin boundary condition $k \nabla T \cdot \mathbf{n} + \beta T_b = \gamma$,
implemented by inheriting from `LinearFVAdvectionDiffusionFunctorRobinBCBase` with:

- $\alpha = k$ (`diffusion_coeff`)
- $\beta = 4 \sigma \varepsilon T_{b,\text{old}}^3$
- $\gamma = \sigma \varepsilon (3 T_{b,\text{old}}^4 + T_\infty^4)$

The Robin base class handles all matrix and RHS contributions, including non-orthogonal
mesh corrections. Using $T_{b,\text{old}}$ (face) instead of $T_{P,\text{old}}$ (cell center)
eliminates the $O(h)$ truncation error in the flux, restoring **second-order spatial accuracy**.

!alert note
The `diffusion_coeff` parameter must match the diffusion coefficient used in the
`LinearFVDiffusion` kernel, as it is the $\alpha$ coefficient in the Robin formulation.

## Picard Convergence

Because $\beta$ and $\gamma$ depend on $T_{b,\text{old}}$ from the previous iteration,
the linear system must be reassembled each outer iteration with the updated solution.
This is achieved by:

- **Pseudo-transient stepping**: use `LinearFVTimeDerivative` + `Transient` executioner
  with large time steps. The matrix is rebuilt at each step with the current $T_\text{old}$.
- **Coupled nonlinear solve**: if the temperature variable is coupled to a nonlinear
  system, the NL solver drives the Picard iteration automatically.

!syntax parameters /LinearFVBCs/LinearFVFunctorRadiativeBC

!syntax inputs /LinearFVBCs/LinearFVFunctorRadiativeBC

!syntax children /LinearFVBCs/LinearFVFunctorRadiativeBC
