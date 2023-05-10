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

!listing input=tests/misc/save_in/block-restricted-save-in.i block=Outputs

!listing input=tests/outputs/debug/show_var_residual_norms_debug.i block=Debug

We will first go over shared reasons for lack of convergence,
then focus on the linear solve, and finally on the nonlinear solve.

## Both or either nonlinear and linear solves fail

### Poor initial condition label=ic

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

For example, with a variable `u` causing convergence issues,
we can simply limit the rate of change of `u` over the first few seconds of the transient problem.

```
[Kernels]
  ... # regular kernels for the physics of interest (except time derivative for u)
  [initialization_slow_derivative]
    type = FunctorTimeDerivative
    variable = u
    factor = '100 * exp(-10 * t) + 1'
  []
[]
```

This is similar to using smaller time steps at the beginning of the transient! For example with an [IteratiiveAdaptiveDT.md]:

!listing input=test/tests/time_steppers/iteration_adaptive/adapt_tstep_grow_init_dt.i block=TimeStepper

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

- turning off **temporarily** some physics through the [Controls](syntax/Controls/index.md) system. Kernels can be
  turned on/off over part of the simulation as needed. In the example below, we turn on the `Diff0` kernel from 0s to 0.49s
  then turn it off and turn on the `Diff1` kernel.

!listing tests/controls/time_periods/kernels/kernels.i block=Kernels Controls

- MultiApps


## Bad mesh

MOOSE works best on well-meshed geometries.

- If a working solver stops converging when you use it with a new mesh, you should consider that there may be issues with the mesh.
  A list of common issues and features in a mesh that are not supported may be found [here](syntax/Mesh/index.md#examining).
- If you have never used the solver before, you should first try it first on a [MOOSE-generated mesh](syntax/Mesh/index.md).
  If it does not converge there, using a simple mesh will simplify the troubleshooting steps you can find on this page.

## Failing linear solve

### Bad linear convergence

If for a specific time step your linear iterations are not dropping such that it takes many linear iterations to reach `l_tol` or you may never reach `l_tol` because you hit the `l_max_hit`, your preconditioner is not working for the problem.

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

In this case you are likely to require many nonlinear iterations as well, but the reason is that your linear iterations don't drop. This could be due to missing terms or errors in your Jacobian or because the way you are applying your preconditioner in PETSc is not good for the problem. Make sure your Jacobian is correct and add off-diagonal terms for multivariable problems.

!alert note title=To check that your Jacobian is correct

AD & Jacobian


An additional possible reason for a poor linear solve is that your problem is very poorly
conditioned. For diagnosing and combating ill-conditioned systems, please see
[NonlinearSystemBase.md#scaling].

SVD

Eigenvalues


### Parallelism issues label=parallel

If the linear solver converges without issue in serial, but fails in parallel


### Solver reports the presence of a 'not a number' (NaN)



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
the [!param](/Executioner/steady/line_search) parameter. Unfortunately, the default used in MOOSE does not converge for every
problem, and it is regularly necessary to turn the line search off as shown in the example below.

```
[Executioner]
  ...

  line_search = 'none'
[]
```

### Bad nonlinear convergence

If your linear iterations are dropping fine but it takes lots of nonlinear iterations, then your problem is very nonlinear and it is just hard to solve. In this case, you should decrease the time step. However, if you have a multivariable problem, the two residuals may have very different magnitudes, which will make the system hard to solve. Print the nonlinear residuals using the [!param](Debug/show_var_residual_norms) parameter to check their relative magnitudes at the end of a solve. If they are more than an order of magnitude off, then use the `scaling` parameter in the variables block to scale the smaller variable up.

!alert note
Automatic scaling, offered through the [!param](/Executioner/Steady/automatic_scaling) parameter, will often suffice to
handle scaling issues, e.g. normalize desired residual convergence, between equations.

