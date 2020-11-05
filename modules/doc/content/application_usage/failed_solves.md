# Troubleshooting Failed Solves

If your solve does not converge, i.e. you exceed the maximum number of nonlinear iterations (`max_nl_its`), the time step gets cut. If this occurs, you will eventually reach the minimum time step and the solve will fail:

```
Time Step  1, time = 125
                dt = 2e-8
 0 Nonlinear |R| = 6.202666e+03
 1 Nonlinear |R| = 9.602686e+02
 2 Nonlinear |R| = 7.814158e+02
 3 Nonlinear |R| = 5.736782e+02
 4 Nonlinear |R| = 4.872949e+02
 5 Nonlinear |R| = 4.420177e+02
 6 Nonlinear |R| = 4.024829e+02
 7 Nonlinear |R| = 3.664817e+02
 8 Nonlinear |R| = 3.341195e+02
 9 Nonlinear |R| = 3.052765e+02
10 Nonlinear |R| = 2.795248e+02
 Solve Did NOT Converge!

Time Step  1, time = 100
                dt = 1e-8
 0 Nonlinear |R| = 6.202666e+03
 1 Nonlinear |R| = 9.600767e+02
 2 Nonlinear |R| = 7.746469e+02
 3 Nonlinear |R| = 5.609970e+02
 4 Nonlinear |R| = 4.766359e+02
 5 Nonlinear |R| = 4.323344e+02
 6 Nonlinear |R| = 3.935847e+02
 7 Nonlinear |R| = 3.584233e+02
 8 Nonlinear |R| = 3.267590e+02
 9 Nonlinear |R| = 2.985230e+02
10 Nonlinear |R| = 2.733081e+02
 Solve Did NOT Converge!

*** ERROR ***
Solve failed and timestep already at or below dtmin, cannot continue!
```

Your solve may be failing for various reasons:

## Small initial tolerance

If you are running a simulation in which for a specific time step the initial tolerance begins very small (>1e-6), your solve fails simply because the `nl_rel_tol` would force te residual too small to reach.

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

You may be in a close to steady-state regime such that the previous solution is very close to the current solution. In this case, setting a `nl_abs_tol` will fix your problem.

## Bad linear convergence

If for a specific time step your linear iterations are not dropping such that it takes many linear iterations to reach `l_tol` or you may never reach `l_tol` because you hit the `l_max_hit`, your preconditioner is not working for the problem.

```
Initial residual before setting preset BCs: 65444.1
 0 Nonlinear |R| = 6.544408e+04
      0 Linear |R| = 6.544408e+04
      1 Linear |R| = 5.381557e+04
      2 Linear |R| = 5.381315e+04
      3 Linear |R| = 5.381315e+04
      4 Linear |R| = 5.381315e+04
      5 Linear |R| = 5.381315e+04
      6 Linear |R| = 5.381315e+04
      7 Linear |R| = 5.381315e+04
      8 Linear |R| = 5.381315e+04
      9 Linear |R| = 5.381315e+04
     10 Linear |R| = 5.381315e+04
     11 Linear |R| = 5.381315e+04
     12 Linear |R| = 5.381315e+04
     13 Linear |R| = 5.381315e+04
     14 Linear |R| = 5.381315e+04
     15 Linear |R| = 5.381315e+04
 1 Nonlinear |R| = 5.510740e+04
      0 Linear |R| = 5.510740e+04
      1 Linear |R| = 5.510740e+04
      2 Linear |R| = 5.510738e+04
      3 Linear |R| = 5.510737e+04
      4 Linear |R| = 5.510735e+04
      5 Linear |R| = 5.510734e+04
      6 Linear |R| = 5.510732e+04
      7 Linear |R| = 5.510730e+04
      8 Linear |R| = 5.510729e+04
      9 Linear |R| = 5.510727e+04
     10 Linear |R| = 5.510726e+04
     11 Linear |R| = 5.510724e+04
     12 Linear |R| = 5.510722e+04
     13 Linear |R| = 5.510721e+04
     14 Linear |R| = 5.510719e+04
     15 Linear |R| = 5.510718e+04
 Solve Did NOT Converge!
```

In this case you are likely to require many nonlinear iterations as well, but the reason is that your linear iterations don't drop. This could be due to missing terms or errors in your Jacobian or because the way you are applying your preconditioner in PETSc is not good for the problem. Make sure your Jacobian is correct and add off-diagonal terms for multivariable problems.

An additional possible reason for a poor linear solve is that your problem is very poorly
conditioned. For diagnosing and combating ill-conditioned systems, please see
[NonlinearSystemBase.md#scaling].

## Bad nonlinear convergence

If your linear iterations are dropping fine but it takes lots of nonlinear iterations, then your problem is very nonlinear and it is just hard to solve. In this case, you should decrease the time step. However, if you have a multivariable problem, the two residuals may have very different magnitudes, which will make the system hard to solve. Print the nonlinear residuals using the debug block to check their relative magnitudes at the end of a solve. If they are more than an order of magnitude off, then use the `scaling` parameter in the variables block to scale the smaller variable up.
