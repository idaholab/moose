# Troubleshooting Failed Solves

!alert note
Before troubleshooting, it is a good idea to make a copy of the input so you do not accidentally leave
troubleshooting objects in the final working version of the input!

If your solve does not converge, i.e. you exceed the maximum number of nonlinear iterations (`max_nl_its`), the time step gets cut.
If this occurs repeatedly, you will eventually reach the minimum time step and the solve will fail:

```
Time Step  1, time = 100
                dt = 2e-8
 0 Nonlinear |R| = 6.202666e+03
 1 Nonlinear |R| = 6.202666e+03
... (until nl_max_its=10)
10 Nonlinear |R| = 6.202666e+03
 Solve Did NOT Converge!

Time Step  1, time = 100
                dt = 1e-8
 0 Nonlinear |R| = 6.202666e+03
 1 Nonlinear |R| = 6.202666e+03
... (until nl_max_its=10)
10 Nonlinear |R| = 6.202666e+03
 Solve Did NOT Converge!

*** ERROR ***
Solve failed and timestep already at or below dtmin, cannot continue!
```

Your solve may be failing for various reasons. First you should identify if it is the nonlinear or the (nested) linear solve that is
failing. Failed linear solves can make the nonlinear solver fail. Similarly, a failing nonlinear solve can take the current
solution vectors in a domain where the linear solve is completely untractable. Therefore it is really important to
identify the pathway to non-convergence using the [Debug](Debug/index.md) and [Outputs](Outputs/index.md) options.

The following snippets show how to obtain detailed prints of the residual convergence during the linear and nonlinear solves.
Nonlinear residual prints are turned on by default in most applications, but the syntax is similar if not.

!listing test/tests/misc/save_in/block-restricted-save-in.i block=Outputs

!listing test/tests/outputs/debug/show_var_residual_norms_debug.i block=Debug

We will first go over shared reasons for lack of convergence,
then focus on the linear solve, and finally on the nonlinear solve.

## Both or either nonlinear and linear solves fail

### Poor initial condition id=ic

For the nonlinear solve,
Newton's method and Newton-Krylov methods in general are only guaranteed to converge when they are in the region of attraction
of the solution. This means that the current numerical solution is close enough that the Jacobian or the approximation to
the Jacobian, helps produce an update vector that gets the solver nearer to the solution.

For the linear solve, a bad initialization, such as the default `null` initialization, can make the problem ill-conditioned
or even completely out of bounds for the material properties, especially correlation that were only tuned over a physically-reachable range.

The easiest initialization is to add to the `Variables` block a reasonable default for each variable. Depending on the
problem being solved and the numerical scheme, it may be necessary to initialize all the nonlinear `Variables`, and
maybe only a few of the auxiliary `AuxVariables`. For example in this flow simulation:

```
[ICs]
  [velocity_x]
    type = ConstantIC
    initial_condition = 0.01 # m/s
  []
  [pressure]
    type = ConstantIC
    initial_condition = 1e5  # 1 atmosphere in Pa
  []
  [temperature]
    type = ConstantIC
    initial_condition = 300  # K
  []
[]
```

However, if we **cannot** guess a better initialization for the solution, a sound approach to these problems is to relax
or linearize the equations to obtain an initial condition,
then progressively remove this numerical treatment in order to restore the original equation solution and/or
the expected order of numerical convergence. A few of these techniques are implemented very simply as shown below:

- extra time-dependent diffusion

For example, with a variable `u` causing convergence issues,
we can simply add a strong diffusion term over the first few seconds of the transient problem.

```
[Kernels]
  ... # regular kernels for the physics of interest
  [initialization_diffusion]
    type = FunctionDiffusion
    variable = u
    function = '100 * exp(-10 * t)'
  []
[]
```

- pseudo-transient relaxation

Certain classes of problems are easier to solve as transients than trying to compute a steady solve. Other transient
problems are difficult to initialize. These problems can benefit from a customized pseudo-relaxation transient at initialization.

These transients usually benefit from a slow growth in the time steps. For example with an [IterationAdaptiveDT.md],
you can start with a small time step and let the time stepper grow the time step to reach steady state faster.

!listing test/tests/time_steppers/iteration_adaptive/adapt_tstep_grow_init_dt.i block=TimeStepper

For a customized relaxation for each variable, you may multiply the time derivatives of each variable
by a different factor.

- diagonal damping

We can adapt the line search in the Newton method to take slower updates if the nonlinear search.
This can help stabilize a nonlinear solve. As mentioned in the relevant
[PETSc documentation](https://petsc.org/release/manualpages/SNES/SNESLINESEARCHBASIC/), this can be achieved as follows:

```
[Executioner]
  ... # other executioner settings
  line_search = 'basic'
  petsc_options_iname = '-snes_linesearch_damping'
  petsc_options_value = '0.5'
```

- turning off **temporarily** some physics through the [Controls](syntax/Controls/index.md) system. Some equations, such as
  advection, can be singular, have a saddle point, ill-conditioned or simply not in the hyperbolic/elliptic regime that the solver
  can handle with a poor initialization. The equations can be modified dynamically during the simulation. Kernels can be
  turned on/off over part of the simulation as needed. In the example below, we turn on the `Diff0` kernel from 0s to 0.49s
  then turn it off and turn on the `Diff1` kernel.

!listing test/tests/controls/time_periods/kernels/kernels.i block=Kernels Controls

- Use a separate simulation to initialize the problem, then leverage the [Restart system](restart_recover.md)
  or a [SolutionUserObject.md] to load the initial solution into the problem of interest.

- In the same vein, you may use a [MultiApp](syntax/MultiApps/index.md) to compute an initial solution then use
  [Transfers](syntax/Transfers/index.md)
  to move the fields from the initialization simulation to the main simulation. This is a straightforward setup, allowing
  for completely different simulations to be used for initialization, with for example a different mesh, a different equation,
  a different equation type (eigenvalue vs. transient for example). The `MultiApps` is then typically executed with
  [!param](/MultiApps/TransientMultiApp/execute_on) set to `INITIAL`.


### Bad mesh

MOOSE works best on well-meshed geometries. If your mesh is bad, your results will be bad. It is the fundamental law of
`garbage-in-garbage-out`.

- If a working solver stops converging when you use it with a new mesh, you should consider that there may be issues with the mesh.
  A list of common issues and features in a mesh that are not supported may be found [here](syntax/Mesh/index.md#examining).
- If you have never used the solver before, you should first try it first on a [MOOSE-generated mesh](syntax/Mesh/index.md).
  If it does not converge there, it will not converge on a complex mesh and using a simple mesh will simplify the troubleshooting
  steps you can find on this page.

## Failing linear solve

If for a specific time step your linear iterations are not dropping such that it takes many linear iterations to reach `l_tol` or you may never reach `l_tol` because you hit the `l_max_it`, your preconditioner is not working for the problem.

```
Initial residual before setting preset BCs: 65444.1
 0 Nonlinear |R| = 6.544408e+04
      0 Linear |R| = 6.544408e+04
      1 Linear |R| = 5.381557e+04
      2 Linear |R| = 5.381315e+04
      3 Linear |R| = 5.381315e+04
... (until l_max_its=15)
     15 Linear |R| = 5.381315e+04
 1 Nonlinear |R| = 5.510740e+04
      0 Linear |R| = 5.510740e+04
      1 Linear |R| = 5.510740e+04
      2 Linear |R| = 5.510738e+04
      3 Linear |R| = 5.510737e+04
... (until l_max_its=15)
     15 Linear |R| = 5.381315e+04
 Solve Did NOT Converge!
```

In this case you are likely to require many nonlinear iterations as well, but the reason is that your linear iterations don't drop. This could be due to missing terms or errors in your Jacobian -- especially for Newton's method, but for PJFNK the Jacobian is often used for preconditioning still -- or because the way you are applying your preconditioner in PETSc is not good for the problem. First, let's make sure your Jacobian is correct and has off-diagonal terms for multivariable problems.

!alert note
Additional resources on linear solve issues may be found in the
[PETSc manual FAQ](https://petsc.org/release/faq)

### Incorrect Jacobian

First, it is important to understand where the Jacobian may come into play because this sets the bar for how accurate the
Jacobian needs to be.
For Newton's method, the linear solve is essentially the inversion of the Jacobian (an explicit inverse is generally **NOT** formed)
and its action on the residual vector. As such, we generally consider that it is **essential** that the Jacobian be accurate.
A solve with a poor Jacobian is unlikely to converge.
For PJFNK, the linear systems are formed by computing the action of the Jacobian rather than the Jacobian. But the preconditioning
often relies on forming the Jacobian. By default, only the diagonal of the Jacobian is computed for preconditioning. To form
the entire Jacobian, including the variable coupling terms on the off-diagonals, this syntax may be used:

!listing test/tests/preconditioners/auto_smp/ad_coupled_convection.i block=Preconditioning

A basic check on whether the Jacobian may be inaccurate is to simply replace it with a Jacobian formed using finite differencing,
using the [Finite Difference Preconditioner](FiniteDifferencePreconditioner.md). This replaces every call to `::computeJacobian` in the kernels and boundary
conditions with finite differencing on the computation of the residual. If a case of poor convergence is fixed by this
simple action, then the Jacobian was not good and it needs to be fixed.

!listing test/tests/preconditioners/fdp/fdp_test.i block=Preconditioning

A more advanced check, and part of the process to fix the Jacobian, is to use the
[Jacobian analysis utilities](development/analyze_jacobian.md). These will point out inaccurate or missing terms
in the Jacobian. They can diagnose missing variable coupling terms for example. Jacobian deficiencies for small problems may also be investigated from the command line using the PETSc options `-snes_test_jacobian -snes_test_jacobian_view`. Combined with [DOFMapOutput.md] output, this can be similarly used to diagnose problematic areas of the Jacobian. Once the errors have been identified,
it is time to find a pen and paper and re-derive the Jacobian from the weak form of the equations. Either that, or use
[automatic differentiation (AD)](automatic_differentiation/index.md).

!alert note
If you are using code that relies on [automatic differentiation](automatic_differentiation/index.md) for forming
the residual, the Jacobian would only be inexact due to a mistake in the code. It does not hurt to check though. Note
that AD kernels can be mixed with regular kernels.

### Ill-conditioned or ill-posed problem

An additional possible reason for a poor linear solve is that your problem is very poorly
conditioned. For diagnosing and combating ill-conditioned systems, please see
[NonlinearSystemBase.md#scaling]. We explain here two simple techniques to diagnose these woes.

- If you can reduce the size of your problem below 10,000 nonlinear degrees of freedom (1,000 preferred), as reported in the
  simulation log header, you can use a Singular Value Decomposition (SVD) to compute the number of singular values and the condition
  number, the ratio of the largest to smallest singular value. To reduce the
  size of the problem, you should consider using a MOOSE-generated [Cartesian mesh](CartesianMeshGenerator.md).
  You will need to use these options:

```
[Executioner]
  ... (other executioner settings)
  petsc_options = '-pc_svd_monitor'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'svd'
[]
```

If the svd monitor reports **ANY** singular value, your problem is ill-posed. This usually means you forgot a boundary condition,
a subdomain-restriction on a kernel, or that your current solution (or initialization) is at a saddle point.
MOOSE can use PETSc to remove the nullspace and still output a solution, but this is generally not recommended. You need
to work on making sure the Jacobian is correct and the numerical problem is well-posed.

If the svd monitor reports a large condition number, your problem is ill-conditioned. This usually means it will be difficult
to solve numerically, and that the residuals will be hard to reduce evenly across variables and degrees of freedom.
We usually first address this by [scaling equations](NonlinearSystemBase.md#scaling). Further effort on the numerical scheme
and preconditioning will likely be necessary.

- Alternatively, for larger problems, you may consider using eigenvalues instead of singular values to reach similar
  conclusions. To obtain an approximation to the eigenvalues of your problem, you may use these petsc options:

```
-ksp_compute_eigenvalues -ksp_gmres_restart 1000 -pc_type none
```

Note that the larger the number of Krylov vectors before restarting, the more eigenvalues you will be able to compute.
But the memory cost will increase linearly with that number.
Any zero eigenvalue will pose the same problem as a zero singular value.
The size and values of eigenvalues will help you understand which preconditioner are likely to work, based on whether
the problem is elliptic, parabolic or hyperbolic.

### Inaccurate Jacobian action

When using PJFNK with a noisy residual function, the Jacobian action computed via finite differencing of the residuals may be inaccurate. One may use the PETSc command line option `-ksp_monitor_true_residual` to see whether the true residual measured via Ax - b is decreasing (as opposed to the residual generated during GMRES iterations). If the GMRES residual is dropping but the true residual is not, then variables may need to be better scaled. Additionally the user may experiment with the `-mat_mffd_err` parameter. The default value is around 1e-8; however, values nearer 1e-6 or 1e-5 may be better for noisy functions in which a larger differencing may be necessary to cut through the noise.

### Parallelism issues id=parallel

If the linear solver converges without issue in serial, but fails in parallel, this is also likely due to insufficiencies with
the preconditioning. Numerous preconditioners perform different operations when used in parallel. For example, the extent of the
blocks in a Block-Jacobi (`pc_type bjacobi`) preconditioner is tied to the number of processes used. As a result, even if the
solution to the problem is unique, the number of iterations and the convergence history is different.

If your problem is small (<200k dofs for example), you could default to a direct solver, for example with `pc_type lu`,
which generally achieve identical results both in serial and parallel.

For larger problems, you need to carefully consider the preconditioning. Expert knowledge in numerical schemes for the
equations you are trying to solve will facilitate crafting a new scheme to solve them.
The larger the problem, the more you will need to
consider the scalability of the methods under consideration.

!alert note
[Field split preconditioning](FieldSplitPreconditioner.md) is one of the preferred methods of preconditioning multi-variable problems in MOOSE
in a scalable manner.

### Solver reports the presence of a 'not a number' (NaN) id=nan

If the solver is faced with invalid floating point arithmetic, such as dividing by 0 or computing the derivative of a
L2-norm at 0, it will generate a `NaN` in the solution vector. This is undesirable and will cause the solve to fail.
MOOSE will report the nonlinear solve as having not converged with:

```
Nonlinear solve did not converge due to DIVERGED_FNORM_NAN.
```

99% of the time, this arises because a variable is not initialized and the material properties are being evaluated out
of bounds. Initializing all the variables in the simulation will fix the problem.

If not, you must find the source of the `NaN`. In most cases, you will be able to run your simulation again and
pass the command line argument `--trap-fpe`. In some cases, this will not suffice and you will need to use a debugger.
Detailed instructions on using the debugger with a MOOSE-based application can be found on this [page](application_development/debugging.md).
Once you have started the debugger, you will need to set a breakpoint on floating point exceptions then generate a backtrace.

```
  break libmesh_handleFPE
```

In the backtrace generated, you will want to look for the object, or the routine within the object, involved.
From there, you must find a way to avoid the floating point error. For example, in the parsed expression below, we
avoid the division by 0 by setting a minimum denominator.

```
[Materials]
  [my_prone_to_nan_material]
    type = ParsedMaterial
    property_name = 'le_ratio'
    expression = '1 / min(u, 1e-8)'
    coupled_variables = u
  []
[]
```


## Failing nonlinear solve

### Too small initial tolerance

If you are running a simulation in which for a specific time step the initial tolerance begins very small (>1e-6), your solve fails simply because the `nl_rel_tol` would force the residual too small to reach.

```
Initial residual before setting preset BCs: 4.84302e-09
 0 Nonlinear |R| = 4.843024e-09
 1 Nonlinear |R| = 1.273033e-13
 2 Nonlinear |R| = 5.226870e-17
 3 Nonlinear |R| = 2.580131e-17
 4 Nonlinear |R| = 2.522566e-17
 5 Nonlinear |R| = 2.522556e-17
 Solve Did NOT Converge!
```

You may be in a close to steady-state regime such that the previous solution is very close to the current solution. In this case, setting an absolute tolerance (`nl_abs_tol`) or a looser relative tolerance (`nl_rel_tol`) will fix your problem.
An alternative is to turn on [`automatic_scaling`](systems/NonlinearSystemBase.md#auto-scaling) in the same `Executioner` input block.
This will normalize the convergence criteria.

### Line search did not converge

Line searches attempt to compute optimal updates in Newton-Krylov solves. MOOSE gives access to several PETSc line searches using
the [!param](/Executioner/Steady/line_search) parameter. Unfortunately, the default used in MOOSE does not converge for every
problem, and it is regularly necessary to turn the line search off as shown in the example below.

```
[Executioner]
  ...

  line_search = 'none'
[]
```

### Bad nonlinear convergence

If your linear iterations are dropping fine but it takes lots of nonlinear iterations, then your problem is very nonlinear and it is just hard to solve. In this case, you should decrease the time step. However, if you have a multivariable problem, the two residuals may have very different magnitudes, which will make the system hard to solve. Print the nonlinear residuals using the [!param](/Debug/SetupDebugAction/show_var_residual_norms) parameter to check their relative magnitudes at the end of a solve. If they are more than an order of magnitude off, then use the `scaling` parameter in the variables block to scale the smaller variable up.

!alert note
Automatic scaling, offered through the [!param](/Executioner/Steady/automatic_scaling) parameter, will often suffice to
handle scaling issues, e.g. normalize desired residual convergence, between equations.

