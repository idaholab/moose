# PIMPLE

!syntax description /Executioner/PIMPLE

## Overview

This transient executioner is based on the algorithm discussed in [!cite](greenshieldsweller2022).
It is a combination of [SIMPLE.md] and the PISO (Pressure-Implicit with Splitting of Operators)
algorithm introduced by [!cite](issa1986solution).

In PISO, the user iterates between the source term in the pressure equation and the corrected pressure
and velocity fields without assembling the momentum system again. With the notation
already used in [SIMPLE.md], the PISO iteration is the following:

1. Take a velocity guess and compute $H(u^n)$ which is the offdiagonals of the momentum system matrix multiplied by
   the current guess minus the right hand side of the momentum matrix. Note that the face flux is computed using
   a different quess so this operation is just a matrix-vector multiplication and a vector-vector addition.
2. Use $H(u^n)$ and the diagonal of the momentum matrix $A$ to solve the pressure equation:

   !equation
   \nabla \cdot \left(A^{-1}H(\vec{u}^{n})\right) = -\nabla \cdot \left(A^{-1}\nabla p^n\right).

   The pressure solution might have to be relaxed in this iteration:

   !equation
   p^{n,*} = p^n + \lambda_p (p^{n}-p^{n-1}).

3. Once the pressure solution is obtained, update the velocity to the next guess:

   !equation id=pressure-eq-ainv
   \vec{u}^{n+1} = - A^{-1}H(\vec{u}^{n}) -A^{-1}\nabla p^{n,*},

   and return to (1) until the maximum number of iterations is reached which can be set
   using the [!param](/Executioner/PIMPLE/num_piso_iterations) parameter.

## Example Input Syntax

The problem setup is exactly the same as discussed for [SIMPLE.md], only the executioner
block is different:

!listing modules/navier_stokes/test/tests/finite_volume/ins/channel-flow/linear-segregated/2d/2d-boussinesq-transient.i block=Executioner

### Restarting and keeping certain solution fields unchanged

When recovering from a checkpoint, it can be useful to hold the thermal-hydraulics fields fixed
and only advance other systems. The `PIMPLE` executioner exposes flags that selectively disable parts
of the segregated solve:

- [!param](/Executioner/PIMPLE/solve_momentum)
- [!param](/Executioner/PIMPLE/solve_pressure)
- [!param](/Executioner/PIMPLE/solve_energy)
- [!param](/Executioner/PIMPLE/solve_solid_energy)
- [!param](/Executioner/PIMPLE/solve_turbulence)
- [!param](/Executioner/PIMPLE/solve_active_scalars)
- [!param](/Executioner/PIMPLE/solve_passive_scalars)

For example, to load a converged flow/temperature field from a steady-state run and only march
passive scalars, enable [restart and recovery](restart_recover.md), keep the scalar solves enabled, and disable the momentum,
pressure, and energy solves:

```
[Problem]
  restart_file_base=converged_run_cp/LATEST
[]
[Executioner]
  type = PIMPLE
  ...
  solve_momentum = false
  solve_pressure = false
  solve_energy = false
[]
```

!syntax parameters /Executioner/PIMPLE

!syntax inputs /Executioner/PIMPLE

!syntax children /Executioner/PIMPLE
