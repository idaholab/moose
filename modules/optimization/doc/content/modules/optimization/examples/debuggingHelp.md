# Debugging Help

Optimization problems are difficult to set-up correctly because each sub-app must be solving and returning the correct data to TAO.  It is easiest to build up the optimization problem by starting with the simplest optimization algorithms and working up in complexity.  By simplest, I mean the algorithm requiring the least number of sub-apps, i.e. gradient free optimization will be much easier to set-up than gradient and Hessian based optimization problems.  

For problems with only a few parameters, the gradient free Nelder Mead ,`taonm`, algorithm will be the easiest to get running.  This will determine if the forward problem is providing the objective function with the correct information.  The next step is to use a gradient based solver like the TAO limited memory variable metric, `taolmvm`, with a gradient provided from finite differencing instead of an adjoint sub-app.  This can help you determine if bounds or better initial guesses on the parameters are needed.  However, finite differences are slow so this should only be used with a few parameters.  An example of a finite difference derivative provided by TAO using the `petsc_options` is shown in the following `[Executioner]` block  

```language=bash
[Executioner]
 type = Optimize
 tao_solver = taolmvm
 petsc_options_iname='-tao_fd_gradient -tao_fd_delta -tao_gatol'
 petsc_options_value='true 1e-8 0.1'
 verbose = true
[]
```

The `-tao_fd_delta` size of 1e-8 is problem dependent.  

If everything is going well up to this point, its time to use an adjoint sub-app to compute the gradient.  This requires the formulation of the adjoint and gradient and the translation of all this into a MOOSE input file.  If the optimization algorithm stops converging, then something is probably wrong with the gradient being computed by the adjoint-app.  TAO can check the gradient computed by the adjoint problem with that computed using finite differencing with the following `petsc_options`:

```language=bash
[Executioner]
  type = Optimize
  tao_solver = taobncg
  petsc_options_iname='-tao_max_it -tao_fd_test -tao_test_gradient -tao_fd_gradient -tao_fd_delta -tao_ls_type'
  petsc_options_value='1 true true false 1e-8 unit'
  petsc_options = '-tao_test_gradient_view'
  verbose = true
[]
```

Only a single iteration is is taken in this executioner block which is enough to see if the gradient is correct but probably not enough for the problem to converge.  In the above executioner block uses the conjugate gradient (`tooabncg`) solver with the line search turned off  (`-tao_ls_type=unit`) to reduce the number of forward and adjoint solves taken during a single optimization iteration.  Output from the gradient test in the executioner block will look like this when the gradient is correct:

```language=bash
||Gfd|| 1.43932, ||G|| = 1.43924, angle cosine = (Gfd'G)/||Gfd||||G|| = 1.
2-norm ||G - Gfd||/||G|| = 0.000288047, ||G - Gfd|| = 0.000414591
max-norm ||G - Gfd||/||G|| = 0.00028627, ||G - Gfd|| = 0.000386034
```

where the norm `||G||` is for the adjoint gradient and `||Gfd||` is for the finite different gradient.  Individual components for each type of gradient are also printed by including `petsc_options = '-tao_test_gradient_view'`.  The executioner option `verbose = true` provides a summary of the optimization algorithm at the end of the simulation including the number of iterations, norm of the gradient and objective function.  It also includes the reason the optimization algorithm exited.  For a successful simulation, it gives the reason for convergence.  For instance, the TAO default convergence criterion is for the gradient to go below an absolute tolerance as shown by:

```language=bash
 Solution converged:    ||g(X)|| <= gatol
```

A failed optimization solve will also provide a reason the optimization solver terminated.  An example where it reaches the maximum number of optimization iterations is given by

```language=bash
  Solver terminated: -2   Maximum Iterations
```

And finally, an example for a failed line search which is usually caused by an inaccurate gradient or from making the optimization convergence tolerance too small:

```language=bash
  Solver terminated: -6   Line Search Failure
```
