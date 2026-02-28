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

Because the LinearFV framework assembles an explicit linear system, the nonlinear $T^4$
term must be **Newton-linearized** around the current cell-center temperature $T_\text{old}$:

$$
q_\text{out} \approx
  \underbrace{4 \sigma \varepsilon T_\text{old}^3}_{\text{matrix}} \cdot T
  - \underbrace{\sigma \varepsilon \left( 3 T_\text{old}^4 + T_\infty^4 \right)}_{\text{RHS}}
$$

This yields:

- **`computeBoundaryGradientMatrixContribution()`** $= 4 \sigma \varepsilon T_\text{old}^3$
  (positive, improves diagonal dominance)
- **`computeBoundaryGradientRHSContribution()`** $= \sigma \varepsilon (3 T_\text{old}^4 + T_\infty^4)$

The balance at convergence ($T_\text{old} = T_P$) reduces correctly to:

$$
k \frac{T_P - T_N}{d} = \sigma \varepsilon \left( T_P^4 - T_\infty^4 \right) \cdot A_f
$$

confirming that conduction through the domain equals the outward radiative flux.

## Picard Convergence

Because the linearization depends on $T_\text{old}$, the linear system must be reassembled
each outer iteration with the updated solution. This is achieved by:

- **Pseudo-transient stepping**: use `LinearFVTimeDerivative` + `Transient` executioner
  with large time steps. The matrix is rebuilt at each step with the current $T_\text{old}$.
- **Coupled nonlinear solve**: if the temperature variable is coupled to a nonlinear
  system, the NL solver drives the Picard iteration automatically.

!syntax parameters /LinearFVBCs/LinearFVFunctorRadiativeBC

!syntax inputs /LinearFVBCs/LinearFVFunctorRadiativeBC

!syntax children /LinearFVBCs/LinearFVFunctorRadiativeBC
