# Debugging Help

Optimization problems are difficult to set-up correctly because each sub-app must be solving for and returning the correct data to TAO.  It is easiest to build up the optimization problem by starting with the simplest optimization algorithms and working up in complexity.  By simplest, I mean the algorithm requiring the least number of sub-apps, i.e. gradient free optimization will be much easier to set-up than gradient and Hessian based optimization problems.  

For problems with only a few parameters, the gradient-free Nelder Mead ,`taonm`, algorithm will be the easiest to get running.  This will determine if the forward problem is providing the objective function with the correct information.  The next step is to use a gradient based solver like the TAO limited memory variable metric, `taolmvm`, with a gradient provided from finite differencing instead of an adjoint sub-app.  This can help you determine if bounds or better initial guesses on the parameters are needed.  However, finite differences are slow so this should only be used with a few parameters.  An example of a finite difference derivative provided by TAO using the `petsc_options` is shown in the following `[Executioner]` block  

!listing executioners/debug_fd.i block=Executioner

The `-tao_fd_delta` size of 1e-8 is problem dependent.  

If everything is going well up to this point, it's time to use an adjoint sub-app to compute the gradient.  This requires the formulation of the adjoint and gradient and the translation of all this into a MOOSE input file.  If the optimization algorithm stops converging, then something is probably wrong with the gradient being computed by the adjoint-app.  TAO can check the gradient computed by the adjoint problem with that computed using finite differencing with the following `petsc_options`:

!listing executioners/debug_gradient.i block=Executioner

Only a single iteration is taken in this executioner block which is enough to see if the gradient is correct but probably not enough for the problem to converge.  The above executioner block uses the conjugate gradient (`tooabncg`) solver with the line search turned off  (`-tao_ls_type=unit`) to reduce the number of forward and adjoint solves taken during a single optimization iteration.  Output from the gradient test in the executioner block will look like this when the gradient is correct:

!listing executioners/gold/debug_gradient.out start=||Gfd|| end=max-norm ||G - Gfd|| include-end=True

where the norm `||G||` is for the adjoint gradient and `||Gfd||` is for the finite different gradient.  Individual components for each type of gradient are also printed by including `petsc_options = '-tao_test_gradient_view'`.  The executioner option `verbose = true` provides a summary of the optimization algorithm at the end of the simulation including the number of iterations, norm of the gradient and objective function.  It also includes the reason the optimization algorithm exited.  For a successful simulation, it gives the reason for convergence.  For instance, the TAO default convergence criterion is for the gradient to go below an absolute tolerance as shown by:

!listing executioners/gold/debug_gradient.out start=Solution converged

A failed optimization solve will also provide a reason the optimization solver terminated.  An example where it reaches the maximum number of optimization iterations is given by

!listing executioners/gold/failed_max_its.out start=Solver terminated

And finally, an example for a failed line search which is usually caused by an inaccurate gradient or from making the optimization convergence tolerance too small:

!listing executioners/gold/failed_ls.out start=Solver terminated
