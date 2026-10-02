# RigidBodyLoadControl

!syntax description /ScalarKernels/RigidBodyLoadControl

## Description

Closes the [load-controlled rigid-body contact](modules/contact/rigid_contact/theory.md#load-control)
system with the integrated-reaction constraint

!equation
R_s = \sum_{i} w_i\, \lambda_i\, (\hat{n}_i \cdot \hat{a}) - F(t) = 0,

where $F(t)$ is the user's target reaction, $\hat{a}$ is
[!param](/ScalarKernels/RigidBodyLoadControl/direction), and $w_i$ come
from a companion [NodalArea](/userobjects/NodalArea.md) UO.  The kernel
also emits the $(\lambda_i, s)$ transpose block on the gap branch of
the min-NCP so the Jacobian is consistent.

## Preconditioning shift

The exact $\partial R_s/\partial s$ is structurally zero in this
formulation; leaving the $(s, s)$ diagonal at zero relies on PETSc's
pivot shift for the direction and produces Newton overshoot on flat
contact patches.  An optional preconditioning shift on the $(s, s)$
diagonal can be supplied in either of two mutually-exclusive forms:

- [!param](/ScalarKernels/RigidBodyLoadControl/kss_stiffness) ---
  constant scalar (default `0`, i.e. disabled).  A sensible starting
  value is the deformable body's Young's modulus.
- [!param](/ScalarKernels/RigidBodyLoadControl/kss_stiffness_function)
  --- function of time, evaluated at the current time on every Jacobian
  assembly.  Useful when a single constant does not cover the range
  spanned by the load history.

The shift affects only the Jacobian; the residual is unchanged, so the
physical fixed point $R_s = 0$ is unchanged.

## Uzawa mode switching

The kernel exposes `setMode(Mode, s_pin)` and a `currentReactionMinusF()`
accessor so that the [UzawaTransient.md] outer executioner can flip it
between:

- `ForceBalance` (default) --- the physical formulation above.
- `PinScalar` --- emits $R_s = s - s_{\text{pin}},\ K_{ss} = 1$ and no
  coupling blocks; used during the inner primal solve so the outer
  loop can hold $s$ fixed.

The `mode` input parameter sets the *starting* mode; runtime updates go
through `setMode()`.

## Parameters

!syntax parameters /ScalarKernels/RigidBodyLoadControl
