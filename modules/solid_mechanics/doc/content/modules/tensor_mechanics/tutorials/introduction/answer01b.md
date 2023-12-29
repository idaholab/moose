> Before you run this first input file take a moment to think about what you
> expect to happen.

We have specified no boundary conditions and no loading. No forces act on any of
the elements. All initial conditions default to zero and we only have
displacement variables in the system.

The zero displacement initial condition should already be a solution to the
given problem. We expect the residual norm to be zero and each timestep to
instantly converge.

#### Running the input

If you have not done so already, compile the `tensor_mechanics-opt` executable by running `make -j N` (where `N` is the number of CPU cores you have available) in `moose/modules/tensor_mechanics/`. Then run the first tutorial input by typing

```
./tensor_mechanics-opt -i tutorials/introduction/mech_step01.i
```

You should see some Framework information output, info on your mesh, and the non-linear system that is being solved, followed by the convergence history:

```
Time Step 0, time = 0

Time Step 1, time = 1, dt = 1
 0 Nonlinear |R| = 0.000000e+00
 Solve Converged!

Time Step 2, time = 2, dt = 1
 0 Nonlinear |R| = 0.000000e+00
 Solve Converged!

Time Step 3, time = 3, dt = 1
 0 Nonlinear |R| = 0.000000e+00
 Solve Converged!

Time Step 4, time = 4, dt = 1
 0 Nonlinear |R| = 0.000000e+00
 Solve Converged!

Time Step 5, time = 5, dt = 1
 0 Nonlinear |R| = 0.000000e+00
 Solve Converged!
```
