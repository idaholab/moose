# Solving Phase Field Models

Once you have developed a phase field model using implicit time integration, you need to solve your
system of nonlinear equations using MOOSE. MOOSE uses the
[PETSc package](http://www.mcs.anl.gov/petsc/documentation/) to solve the equations, therefore PETSc
options that we have found to work well for phase field modeling are provided here.

## Solution Methods

Three solve methods are available in MOOSE, which are set in the [Executioner](Executioner/index.md)
block using the +solve_type+ parameter, where the three possible options are:

- `NEWTON` - Direct solution of the system of equations using Newton's method.
  The full and accurate Jacobian is required. Thus, the [Preconditioner](Preconditioning/index.md)
  block must be employed for systems with multiple nonlinear variables.
- `JFNK` - The system is solved using Jacobian Free Newton Krylov (JFNK), so no
  Jacobian terms are needed. However, JFNK often does not perform well without
  preconditioning.
- `PJFNK` - The system is solved using preconditioned JFNK. The Jacobian is used
  to precondition the matrix, but it does not have to be fully correct and can
  neglect off-diagonal terms.

# Preconditioning Options

When using NEWTON or PJFNK, PETSc is used to manipulate the Jacobian/preconditioner matrix. The
method used for this manipulation is defined using PETSc options in the Executioner
block. Determining the optimal approach can be difficult, but we have identified certain options that
work well for phase field problems. They are summarized below:

### LU

This uses LU decomposition to directly apply the inverse of the Jacobian matrix.  It is typically the
most accurate approach, but is expensive and does not scale well beyond tens of processors. Parallelization requires
_SUPERLU_ to be compiled into PETSc (as provided by the default [installation of MOOSE](getting_started/installation/index.md optional=True)).
It is often a good debugging tool. Typical options in the block are

```
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
```

### ASM

The Additive Schwartz Method (ASM), is a domain decomposition method. It works well for most models,
and is the only method that works well with the split Cahn-Hilliard equations. Increasing the
`-pc_asm_overlap` improves performance but increases the computational expense. Typical options are

```
  petsc_options_iname = '-pc_type -ksp_grmres_restart -sub_ksp_type -sub_pc_type -pc_asm_overlap'
  petsc_options_value = 'asm      31                  preonly       lu           2'
```

#### ASM/ILU

The incomplete factorization method (ILU) allows specifying a fill factor using `-pc_factor_levels`
increasing this can improve the preconditioner. This is the default preconditioner and works well for
elliptic problems such as the pahse field equations.

### BoomerAMG

BoomerAMG is the Hypre implementation of the Algebraic MultiGrid Method. It works well for solving
the Allen-Cahn equation and the direct solution of the Cahn-Hilliard method. It has horrible
performance with solving the split Cahn-Hilliard equations.  Common options are

```
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart -pc_hypre_boomeramg_strong_threshold'
  petsc_options_value = 'hypre    boomeramg      31                 0.7'
```
