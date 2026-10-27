# Generalized Midpoint Rule

## Overview id=overview

By default the [strain calculator](Kinematics.md) evaluates the deformation gradient at the end of the
time step (backward Euler).  The `alpha` parameter on
[`ComputeLagrangianStrain`](ComputeLagrangianStrain.md) generalizes this to a *generalized midpoint
rule*, sampling the deformation gradient -- and therefore the stress and the internal force -- at a
weighted point within the step.  It is an accuracy-versus-robustness choice for the time integration of
the deformation gradient and rarely needs to be changed.

## Formulation id=formulation

The strain calculator forms the generalized-midpoint deformation gradient
\begin{equation}
   F^{\alpha}_{iJ} = \delta_{iJ}
     + \alpha\,\frac{\partial u_i^{(n+1)}}{\partial X_J}
     + (1-\alpha)\,\frac{\partial u_i^{(n)}}{\partial X_J},
\end{equation}
where $(n)$ and $(n+1)$ denote the previous and current steps and $\alpha \in [0.5, 1]$.  The
constitutive model is evaluated at $F^{\alpha}$, so the internal force in the
[balance of linear momentum](BalanceOfLinearMomentum.md) is sampled at this weighted configuration and
the displacement Jacobian carries the factor
$\partial F^{\alpha}_{iJ}/\partial(\partial u_k^{(n+1)}/\partial X_L) = \alpha\,\delta_{ik}\delta_{JL}$.
The previous-step contribution is present only in a transient simulation; a Steady run takes
$F^{(n)} = I$.

## Choosing alpha id=choosing

- `alpha = 1.0` (default): backward Euler.  The deformation gradient is taken fully at the end of the
  step -- first-order accurate over the step and the most robust choice.
- `alpha = 0.5`: the midpoint rule.  Second-order accurate over the step, and matches the implicit
  integration used by Abaqus.

Intermediate values interpolate between the two; values below $0.5$ are not permitted.  Set the
parameter on the strain calculator:

```
[Materials]
  [strain]
    type = ComputeLagrangianStrain
    # ... displacements, base_name, etc.
    alpha = 0.5
  []
[]
```

The choice is independent of the [kinematic approximation](KinematicApproximations.md) and the
[$\bar{F}$ stabilization](Stabilization.md).
