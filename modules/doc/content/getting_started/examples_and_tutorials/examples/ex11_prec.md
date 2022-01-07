# Example 11 : Preconditioning

For a detailed discussion on preconditioning in MOOSE, see [syntax/Preconditioning/index.md].

To summarize, an accurate and complete preconditioning matrix can be important for an efficient Preconditioned Jacobian-Free Newton-Krylove (PJFNK) solve. It's absolutely essential for `solve_type = NEWTON`. MOOSE has ultiple options for buiding the preconditioning matrix :

- (Default) Block Diagonal Preconditioning
- Single Matrix Preconditioner (SMP)
- Finite Difference Preconditioner (FDP)
- Physics Based Preconditioner (PBP)

Block diagonal preconditioning uses kernels' and integrated boundary conditions' `computeQpJacobian` methods to build a block diagonal matrix. It will +not account for variable coupling+. This is the default if a user does not specify a `[Preconditioning]` block in their input file.

The Single Matrix Preconditioner builds its matrix using kernels' and integrated BCs' `computeQpJacobian` +and+ `computeQpOffDiagJacobian` methods, the latter of which is responsible for the contributions of coupled variables. A good, simple example of a `computeQpOffDiagJacobian` method is in [CoupledForce](https://github.com/idaholab/moose/blob/devel/framework/src/kernels/CoupledForce.C). `CoupledForce` contributes a waek-form residual equal to

$R_i = -\psi_i v$

To determine the corresponding off-diagonal Jacobian contribution, one must take the deriative:

$\frac{\partial R_i}{\partial v_j} = -\psi_i \frac{\partial v}{\partial v_j} = -\psi_i \phi_j$

To make use of user-specified `computeQpOffDiagJacobian` methods, one should specify in his/her input file:

```
[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]
```

The user may also choose to omit certain off-diagonal entries from their `SMP` matrix; this is outlined in the detailed [Preconditioning article](http://mooseframework.org/wiki/MooseSystems/Preconditioners/).

To build a preconditioning matrix through finite differencing of the residuals, the user can specify in his/her input file:

```
[Preconditioning]
  [./fdp]
    type = FDP
    full = true
  [../]
[]
```

This will create a near-perfect preconditioning matrix; however, it is extremely slow and will only work in serial. `FDP` should only be used for debugging purposes.



## Complete source files

[default.i](https://github.com/idaholab/moose/blob/devel/examples/ex11_prec/default.i)

[smp.i](https://github.com/idaholab/moose/blob/devel/examples/ex11_prec/smp.i)

[fdp.i](https://github.com/idaholab/moose/blob/devel/examples/ex11_prec/fdp.i)

[CoupledForce.h](https://github.com/idaholab/moose/blob/devel/framework/include/kernels/CoupledForce.h)

[CoupledForce.C](https://github.com/idaholab/moose/blob/devel/framework/src/kernels/CoupledForce.C)

!content pagination use_title=True
                    previous=examples/ex10_aux.md
                    next=examples/ex12_pbp.md
