# RigidBodyNodalNCPKernel

!syntax description /NodalKernels/RigidBodyNodalNCPKernel

## Description

Assembles the pointwise min-NCP normal-contact constraint
$\min(c\,g_i,\ \lambda_i) = 0$ at each node on the contact sideset, using
the signed distance from the level-set contactor.  The kernel emits
consistent Jacobian blocks on both branches (gap and multiplier), plus
--- when the [load-control layer](modules/contact/rigid_contact/theory.md#load-control)
is active --- the $(\lambda_i, s)$ transpose block that the base
`NodalKernel` framework has no hook for.

See the [theory page](modules/contact/rigid_contact/theory.md#normal-contact-as-a-nodal-complementarity-condition)
for the derivation.

## Parameters

!syntax parameters /NodalKernels/RigidBodyNodalNCPKernel
