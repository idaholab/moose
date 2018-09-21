<!-- MOOSE Documentation Stub: Remove this when content is added. -->

# Preconditioning System

## Overview

- Krylov methods need preconditioning to be efficient (or even effective!).
- Reduces the total number of linear iterations.
- Each linear iteration in MOOSE includes a residual evaluation (`computeQpResidual`).
- Krylov methods, in theory, converge in the number of linear iterations equal to the number of unknown in the system.
- Even though the Jacobian is never formed, JFNK methods still require conditioning.
- MOOSE's automatic (without user intervention) preconditioning is fairly minimal.
- Many options exist for implementing improved preconditioning in MOOSE.

## Preconditioned JFNK

- Using right preconditioning, solve

\begin{equation*}
\mathbf{J} (\vec{u}_i) \mathbf{M}^{-1} (\mathbf{M}\delta \vec{u}_{i+1}) = -\vec{R}(\vec{u}_i)
\end{equation*}

- $\mathbf{M}$ symbolically represents the preconditioning matrix or process.
- Inside GMRES, we only apply the action of $\mathbf{M}^{-1}$ on a vector.
- Recall the *unpreconditioned* JFNK approximation  $\mathbf{M}^{-1} = \mathbf{I}$:

\begin{equation*}
\mathbf{J} (\vec{u}_i) \vec{v} \approx \frac{\vec{R}(\vec{u}_i + \epsilon \vec{v}) - \vec{R}(\vec{u}_i)}{\epsilon}
\end{equation*}

- Compare to the right-preconditioned, matrix-free version:

\begin{equation*}
\mathbf{J} (\vec{u}_i) \mathbf{M}^{-1}\vec{v} \approx \frac{\vec{R}(\vec{u}_i + \epsilon \mathbf{M}^{-1}\vec{v}) - \vec{R}(\vec{u}_i)}{\epsilon}
\end{equation*}

## Preconditioning Matrix vs Process

- In the previous section, $\mathbf{M}$ represented the "Preconditioning Matrix".
- The action of $\mathbf{M}^{-1}$ on a vector represents the "Preconditioner" or "Preconditioning Process".
- In MOOSE the "matrix to build" and the "process to apply" with that matrix are separated.
- These are several different ways to build preconditioning matrices:

  - Default: Block Diagonal Preconditioning
  - Single Matrix Preconditioner (SMP)
  - Finite Difference Preconditioner (FDP)
  - Physics Based Preconditioner (PBP)
  - Field Split Preconditioner

- After selecting how to build a preconditioning matrix you can then use solver options to select how to apply the Preconditioner.

## Solve Type

- The default `solve_type` for MOOSE is "Preconditioned JFNK".
- An alternative `solve_type` can be set through either the `[Executioner]` or `[Preconditioner/*]` block.
- Valid options include :

  - `PJFNK` (default)
  - `JFNK`
  - `NEWTON`
  - `FD` (Finite Difference)

## PETSc Preconditioning Options

- For specifying the preconditioning process we use solver options directly (i.e. PETSc options).
- Currently the options for preconditioning with PETSc are exposed to the applications.
- This will change in the future... there will be more generic ways of specifying preconditiong parameters.
- The best place to learn about all of the preconditioning options with PETSc is the user manual.
- We use command-line syntax, but provide places to enter it in the input file.

[http://www.mcs.anl.gov/petsc/petsc-current/docs/manual.pdf](http://www.mcs.anl.gov/petsc/petsc-current/docs/manual.pdf)

## PETSc specific Executioner Options

|`petsc_options` | Description|
|:- | :- |
| `-snes_ksp_ew` | Variable linear solve tolerance, useful for transient solves |
| `-help` | Show PETSc options during the solve |

[](---)

| `petsc_options_iname` | `petsc_options_value` | Description |
| :-                    | :-                    | :- |
| `-pc-type`            | `ilu`                 | Default for serial |
|                       | `bjacobi`             | Default for parallel with `-sub_pc_type ilu` |
|                       | `asm`                 | Additive Schwartz with `-sub_pc_type ilu` |
|                       | `lu`                  | Full LU, serial only |
|                       | `gamg`                | Generalized Geometric-Algebric MultiGrid |
|                       | `hypre`               | Hypre, usually used with `boomeramg` |
| `-sub_pc_type`        | `ilu, lu, hypre`      | Can be used with `bjacobi` or `asm` |
| `-pc_hypre_type`      | `boomeramg`           | Algebraic Multigrid |
| `-ksp_gmres_restart`  | # (default = 30)      | Number of Krylov vectors to store |

## Default Preconditiong Matrix

- Consider the fully-coupled system of equations:

\begin{equation*}
-\nabla \cdot k(T,s) \nabla T  = 0
\end{equation*}

\begin{equation*}
- \nabla \cdot D(T,s) \nabla s  = 0
\end{equation*}

- Fully-coupled Jacobian approximation

\begin{equation*}
    \mathbf{J}(T,s) =
    \begin{bmatrix}
      \frac{\partial (R_T)_i}{\partial T_j} & \frac{\partial (R_T)_i}{\partial s_j} \\
      \frac{\partial (R_s)_i}{\partial T_j} & \frac{\partial (R_s)_i}{\partial s_j}
    \end{bmatrix}
    \approx
    \begin{bmatrix}
      \frac{\partial (R_T)_i}{\partial T_j} & \mathbf{0} \\
      \mathbf{0} & \frac{\partial (R_s)_i}{\partial s_j} \\
    \end{bmatrix}
\end{equation*}

- For our example:

\begin{equation*}
    \mathbf{M} \equiv
    \begin{bmatrix}
      (k(T,s) \nabla \phi_j, \nabla \psi_i) & \mathbf{0} \\
      \mathbf{0} & (D(T,s) \nabla \phi_j, \nabla\psi_i)
    \end{bmatrix} \approx \mathbf{J}(T,s)
\end{equation*}

- This simple style of throwing away the off-diagonal blocks is the way MOOSE will precondition when using the default `solve_type`.

## The Preconditioning Block

```puppet
[Preconditioning]
  active = 'my_prec'

  [./my_prec]
    type = SMP
    #SMP Options Go Here!
    #Override PETSc Options Here!
  [../]

  [./other_prec]
    type = PBP
    #PBP Options Go Here!
    #Override PETSc Options Here!
  [../]
[]
```

- The `Preconditioning` block allows you to define which type of preconditioning matrix to build and what process to apply.
- You can define multiple blocks with diffrent names, allowing you to quckly switch out preconditioning options.
- Each sub-block takes a `type` parameter to specify the type of preconditioning matrix.
- Within the sub-blocks you can also provide other options specific to that type of preconditioning matrix.
- You can also override PETSc options here.
- Only one block can be active at a time.

## Single Matrix Preconditioning (SMP)

- Single Matrix Preconditioner (SMP) builds one preconditioning matrix.
- You enable SMP with: `type = SMP`.
- You specify which block of the matrix to use with:

  ```text
  off_diag_row    = 's'
  off_diag_column = 'T'
  ```

- Which would produce am $\mathbf{M}$ like this:

\begin{equation*}
\mathbf{M} \equiv
    \begin{bmatrix}
      \left(k(T,s) \nabla \phi_j, \nabla \psi_i\right) & \mathbf{0}
      \\[3pt]
      \left(\frac{\partial D(T,s)}{\partial T_j} \nabla s, \nabla \psi_i\right) & \left(D(T,s) \nabla \phi_j, \nabla\psi_i\right)
      \end{bmatrix} \approx \mathbf{J}
\end{equation*}

- In order for this to work you must provide a `computeQpOffDiagJacobian()` function in your Kernels that computes the required partial derivatives.
- To use *all* off diagonal blocks set: `full = true`.

## Finite Difference Preconditioning (FDP)

- The Finite Difference Preconditioner (FDP) allows you to form a "Numerical Jacobian" by doing direct finite differences of your residual statements.
- This is extremely slow and inefficient, but it is a great debugging tool because it allows you to form a nearly perfect preconditioner.
- You specify this by using: `type = FDP`.
- You can use the same options for specifying off-diagonal blocks as SMP.
- Since FDP allows you to build the perfect approximate Jacobian it can be useful to use it directly to solve instead of using JFNK.
- The finite element differencing is sensitive to the differencing parameter whcih can be specified using:

  ```text
  petsc_options_iname = '-mat_fd_coloring_err -mat_fd_type'
  petsc_options_value = '1e-6                 ds'
  ```

!alert note
FDP currently works in serial only! This might change in the future, but FDP will always be meant for debugging purposes!

## Examples

- Default Preconditioning Matrix, Preconditioned JFNK, monitor linear solver, variable linear solver tolerance.
- Use Hypre with algebraic multigrid and store 101 Krylov vectors.

```puppet
[Executioner]
...
petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
petsc_options_value = 'hypre    boomeramg      101'
...
[]
```

- Single Matrix Preconditioner, Fill in the (forced, diffused) block, Preconditioned JFNK, Full inverse with LU

```puppet
  [./SMP_jfnk]
    type = SMP

    off_diag_row    = 'forced'
    off_diag_column = 'diffused'

    petsc_options_iname = '-pc_type'
    petsc_options_value = 'lu'
  [../]
```

See [Example 11](ex11_prec.md)

## Physics Based Preconditioning

- Physics Based Preconditioning is an advanced concept used to more efficiently solve using JFNK.
- The idea is to create a preconditioning process that targets each physics individually.
- In this way you can create a more effective preconditioner... while also maintaining efficiency.
- In MOOSE there is a `PhysicsBasedPreconditioner` object.
- This object allows you to dial up a preconditioning matrix and the operations to be done on the different blocks of that matrix on the fly from the input file.

## What the PBP Does

- The PBP works by partially inverting a preconditioning matrix (usually an approximation of the true Jacobian) by partially inverting each block row in a Block-Gauss-Seidel way.

\begin{equation*}
  \vec{R}(u,v) =
  \begin{bmatrix}
    \vec{r}_u
    \\
    \vec{r}_v
  \end{bmatrix}
\end{equation*}

\begin{equation*}
\mathbf{M} \equiv
\begin{bmatrix}
  \frac{\partial (R_u)_i}{\partial u_j} & \mathbf{0} \\
  \frac{\partial (R_v)_i}{\partial u_j} & \frac{\partial (R_v)_i}{\partial v_j}
\end{bmatrix} \approx \mathbf{J}
\end{equation*}

\begin{equation*}
\mathbf{M} \vec{q} = \vec{p} \quad \Rightarrow \quad
\begin{cases}
\frac{\partial (R_u)_i}{\partial u_j} \vec{q}_u = \vec{p}_u \\[6pt]
\frac{\partial (R_v)_i}{\partial v_j} \vec{q}_v = \vec{p}_v - \frac{\partial (R_v)_i}{\partial u_j} \vec{q}_u
\end{cases}
\end{equation*}

## Using the PBP

```puppet
[Variables]
...
[]

[Preconditioning]
  active = 'myPBP'

  [./myPBP]
    type = PBP
    solve_order = 'u v'
    preconditioner  = 'ILU AMG'
    off_diag_row    = 'v'
    off_diag_column = 'u'
  [../]
[]
```

- Set up a PBP object for a two variable system (consisting of variables "u" and "v").
- Use ILU for the "u" block and AMG for "v".
- Use the lower diagonal (v,u) block.
- When using `type = PBP`, will set `solve_type = JFNK` automatically.

## Applying PBP

Applying these ideas to a coupled thermo-mechanics problem.

!media large_media/preconditioning/precond-conv.png
       style=width:50%;

See [Example 12](ex12_pbp.md)

## Further Preconditiong Documentation

!syntax list /Preconditioning objects=True actions=False subsystems=False

!syntax list /Preconditioning objects=False actions=False subsystems=True

!syntax list /Preconditioning objects=False actions=True subsystems=False

