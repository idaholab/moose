# RigidBodyContactSparsity

!syntax description /UserObjects/RigidBodyContactSparsity

## Description

Preallocates the cross-node (LM, displacement) Jacobian coupling that
[RigidBodyNormalMechanicalContact.md] writes on the lower-d contact block
--- an entry MOOSE's default sparsity computation misses, which without
this object causes PETSc to `malloc` on every Jacobian assembly.  Also
registers a `GhostEverything` relationship manager on the contact face so
that [RigidBodyLoadControl.md] (a `NodalScalarKernel`, called only on the
rank that owns the scalar DoF) can see every LM value it needs to form
the integrated reaction.

Registration is via `SystemBase::addExtraSparsityCallback` so the object
coexists with MOOSE's existing extra-sparsity function slot on the
nonlinear-system DofMap without tripping libMesh's "both function and
object slots set" warning.

Emitted automatically by [RigidContactAction.md]; users rarely construct
it by hand.  The overall role in the formulation is described in the
[theory page](modules/contact/rigid_contact/theory.md#sparsity-augmentation).

## Parameters

!syntax parameters /UserObjects/RigidBodyContactSparsity
