## Preconditioning
---

- Krylov methods need preconditioning to be efficient (or even effective!).
- Even though the Jacobian is never formed, JFNK methods still require preconditioning.
- MOOSE's automatic (without user intervention) preconditioning is fairly minimal.
- Many options exist for implementing improved preconditioning in MOOSE.

## Preconditioned JFNK
---

- Using right preconditioning, solve

\begin{equation} \mathbf{R}'(\mathbf{u}_i)\mathbf{M}^{-1}(\mathbf{M}\delta \mathbf{u}_{i+1}) = -\mathbf{R}(\mathbf{u}_i) \end{equation}

- $\mathbf{M}$ symbolically represents the preconditioning matrix or process
- Inside GMRES, we only apply the action of $\mathbf{M}^{-1}$ on a vector
- Right preconditioned matrix free version

\begin{equation} \mathbf{R}'(\mathbf{u}_i) \mathbf{M}^{-1}\mathbf{v} \approx \frac{\mathbf{R}(\mathbf{u}_i + \epsilon \mathbf{M}^{-1}\mathbf{v}) - \mathbf{R}(\mathbf{u}_i)}{\epsilon} \end{equation}

## Preconditioning Matrix vs Process
---

- On the previous slide $\mathbf{M}$ represented the "Preconditioning Matrix".
- The action of $\mathbf{M}^{-1}$ on a vector represents the "Preconditioner" or "Preconditioning Process".
- In MOOSE the "matrix to build" and the "process to apply" with that matrix are separated.
- There are four different ways to build preconditioning matrices:
    - Default: Block Diagonal Preconditioning
    - Single Matrix Preconditioner (SMP)
    - Finite Difference Preconditioner (FDP)
    - Physics Based Preconditioner (PBP)
- After selecting how to build a preconditioning matrix you can then use solver options to select how to apply the Preconditioner.

## Solve Type
---

- The default `solve_type` for MOOSE is "Preconditioned JFNK".
- An alternative `solve_type` can be set through either the `[Executioner]` or `[Preconditioner/*]` block.
- Valid options include:
    - `PJFNK` (default)
    - `JFNK`
    - `NEWTON`
    - `FD` (Finite Difference)

## PETSc Preconditioning Options
---

- For specifying the preconditioning process we use solver options directly (i.e. PETSc options).
- Currently the options for preconditioning with PETSc are exposed to the applications.
- This will change in the future. . . there will be more generic ways of specifying preconditioning parameters.
- The best place to learn about all of the preconditioning options with PETSc is the user manual.
- We use the command-line syntax, but provide places to enter it into the input file.

<http://www.mcs.anl.gov/petsc/petsc-current/docs/manual.pdf>

## PETSc Specific Options (for Executioner)
---

petsc_option | Description
----------------|-------------
`-snes_ksp_ew` | Variable linear solve tolerance -- useful for transient solves
`-help` | Show PETSc options during the solve

petsc_options_iname | petsc_options_value | Description
------|------|--------
`-pc_type` | `ilu` | Default for serial
| `bjacobi` | Default for parallel with `-sub_pc_type ilu`
| `asm` | Additive Schwartz with `-sub_pc_type ilu`
| `lu` | Full LU, serial only!
| `gamg` | PETSc Geometric AMG Preconditioner
| `hypre` | Hypre, usually used with `boomeramg`
`-sub_pc_type` | `ilu, lu, hypre` | Can be used with bjacobi or asm
`-pc_hypre_type` | `boomeramg` | Algebraic multigrid
`-pc_hypre_boomeramg` (cont.) | | "Information Threshold" for AMG process
` _strong_threshold` | `0.0 - 1.0` | **(0.7 is auto set for 3D)
`-ksp_gmres_restart` | `#` | Number of Krylov vectors to store


## Default Preconditioning Matrix
---

- Consider the fully coupled system of equations:

\begin{equation} - \nabla \cdot k(s,T) \nabla T  = 0 \end{equation}
\begin{equation} - \nabla \cdot D(s,T) \nabla s  = 0 \end{equation}

- Fully coupled Jacobian approximation

\begin{equation}
    \mathbf{R}'(s,T) =
    \begin{bmatrix}
        (\mathbf{R}_T)_T & (\mathbf{R}_T)_S \\
        (\mathbf{R}_S)_T & (\mathbf{R}_S)_S
    \end{bmatrix}
    \approx
    \begin{bmatrix}
        (\mathbf{R}_T)_T & 0 \\
        (0 & (\mathbf{R}_S)_S
    \end{bmatrix}
\end{equation}

- For our example:

\begin{equation}
    \mathbf{M} \equiv
    \begin{bmatrix}
        (k(s,T) \nabla \phi_j, \nabla \psi_i) & \mathbf{0} \\
        \mathbf{0} & (D(s,T) \nabla \phi_j, \nabla\psi_i)
    \end{bmatrix}
    \approx \mathbf{R}'(s,T)
\end{equation}

- This simple style of throwing away the off-diagonal blocks is the way MOOSE will precondition when using the default `solve_type`.

## The Preconditioning Block
---

```pre
[Preconditioning]
  active = 'my_prec'

  [./my_prec]
    type = SMP
    # SMP Options Go Here!
    # Override PETSc Options Here!
  [../]

  [./other_prec]
    type = PBP
    # PBP Options Go Here!
    # Override PETSc Options Here!
  [../]
[]
```

- The `Preconditioning` block allows you to define which type of preconditioning matrix to build and what process to apply.
- You can define multiple blocks with different names, allowing you to quickly switch out preconditioning options.
- Each sub-block takes a `type` parameter to specify the type of preconditioning matrix.
- Within the sub-blocks you can also provide other options specific to that type of preconditioning matrix.
- You can also override PETSc options here.
- Only one block can be active at a time.

## Single Matrix Preconditioning (SMP)
---

- The Single Matrix Preconditioner (SMP) builds one matrix for preconditioning.
- You enable SMP with:

```pre
type = SMP
```

- You specify which blocks of the matrix to use with:

```pre
off_diag_row    = 's'
off_diag_column = 'T'
```

- Which would produce an $\mathbf{M}$ like this:

\begin{equation}
    \mathbf{M} \equiv
    \begin{bmatrix}
        \left(k(s,T) \nabla \phi_j, \nabla \psi_i\right) & \mathbf{0} \\[3pt]
        \left(\frac{\partial D(s,T)}{\partial T_j} \nabla s, \nabla \psi_i\right) & \left(D(s,T) \nabla \phi_j, \nabla\psi_i\right)
    \end{bmatrix}
    \approx \mathbf{R}'
\end{equation}

- In order for this to work you must provide a `computeQpOffDiagJacobian()` function in your Kernels that computes the required partial derivatives.
- To use *all* off diagonal blocks, you can use the following input file syntax:

```pre
full = true
```

## Finite Difference Preconditioning (FDP)
---

- The Finite Difference Preconditioner (FDP) allows you to form a "Numerical Jacobian" by doing direct finite differences of your residual statements.
- This is extremely slow and inefficient, but is a great debugging tool because it allows you to form a nearly perfect preconditioner.
- More information can be found here: <http://mooseframework.org/wiki/JacobianDebugging/>
- You specify it by using:

```pre
type = FDP
```

- You can use the same options for specifying off-diagonal blocks as SMP.
- Since FDP allows you to build the perfect approximate Jacobian it can be useful to use it directly to solve instead of using JFNK.
- The finite differencing is sensitive to the differencing parameter which can be specified using:

```pre
petsc_options_iname = '-mat_fd_coloring_err -mat_fd_type'
petsc_options_value = '1e-6                 ds'
```

## Examples
---

```pre
[Executioner]
  ...
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre    boomeramg      101'
  ...
[]
```

- Default Preconditioning Matrix, Preconditioned JFNK, monitor linear solver, variable linear solver tolerance.
- Use Hypre with algebraic multigrid and store 101 Krylov vectors.

```pre
[Preconditioning]
  active = 'SMP_jfnk'

  [./SMP_jfnk]
    type = SMP

    off_diag_row    = 'forced'
    off_diag_column = 'diffused'

    petsc_options_iname = '-pc_type'
    petsc_options_value = 'lu'
  [../]
[]
```

- Single Matrix Preconditioner, Fill in the (forced, diffused) block, Preconditioned JFNK, Full inverse with LU

 Look at Example 11 (page E)
#
## Physics Based Preconditioning
---

- Physics based preconditioning is an advanced concept used to more efficiently solve using JFNK.
- The idea is to create a preconditioning process that targets each physics individually.
- In this way you can create a more effective preconditioner. . . while also maintaining efficiency.
- In MOOSE there is a `PhysicsBasedPreconditioner` object.
- This object allows you to dial up a preconditioning matrix and the operations to be done on the different blocks of that matrix on the fly from the input file.

## What the PBP Does
---

- The PBP works by partially inverting a preconditioning matrix (usually an approximation of the true Jacobian) by partially inverting each block row in a Block-Gauss-Seidel way.

\begin{equation}
    \mathbf{R}(u,v) =
    \begin{bmatrix}
        \mathbf{R}_u \\
        \mathbf{R}_v
    \end{bmatrix}
\end{equation}

\begin{equation}
    \mathbf{M} \equiv
    \begin{bmatrix}
        (\mathbf{R}_u)_u & \mathbf{0} \\
        (\mathbf{R}_v)_u & (\mathbf{R}_v)_v
    \end{bmatrix}
    \approx \mathbf{R}'
\end{equation}

\begin{equation}
    \mathbf{M} \mathbf{q} = \mathbf{p} \quad \Rightarrow \quad
    \left\{
    \begin{array}{rcl}
    (\mathbf{R}_u)_u \mathbf{q}_u &=& \mathbf{p}_u \\[6pt]
    (\mathbf{R}_v)_v \mathbf{q}_v &=& \mathbf{p}_v - (\mathbf{R}_v)_u \mathbf{q}_u
    \end{array}
    \right.
\end{equation}

## Using the PBP
---

```pre
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
- When using '`type=PBP`', MOOSE will set `solve_type = JFNK` automatically.

## Applying PBP
---

!media media/examples/precond-conv.png width=20% float=right

-  Applying these ideas to a coupled thermo-mechanics problem:
-  Look at [Example 12](examples/Example_12.md)

## Preconditioning System
---

!syntax objects /Preconditioning

!syntax subsystems /Preconditioning

!syntax actions /Preconditioning
