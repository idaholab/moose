# DT2

!syntax description /Executioner/TimeStepper/DT2

This is an adaptive, error-estimate based multi-step time step calculation scheme.
It is meant to provide confidence in the convergence in time step of the results.
Because it takes three time steps for every step, it is a relatively computationally
expensive time stepping scheme.

For each time step, the solution is compared to what the solution would be if two half-time steps
had been taken instead. This latter solution is generally more accurate, so this gives an estimate
of the error. Based on the error, detailed below, and the user set acceptable errors, the time step may be accepted
or rejected.

!equation
error = \dfrac{||u_{dt} - u_{2dt/2}||_{L_2}}{max(||u_{dt}||_{L_2}, ||u_{2dt/2}||_{L_2}) dt}

where $u$ is the solution and $dt$ the time step size.
If the time step is accepted, then the time step is increased to

!equation
dt^{n+1} = dt^n \left(\dfrac{e_{tol}}{error} \right)^{1/order}

where $e_{tol}$ is the user set tolerance on the error and $order$ is the accuracy order
of the time integration scheme.

If the step is rejected, both the non linear and auxiliary systems are reset, and the
time step is reduced before the next attempt.

## Example input syntax

The `DT2` time stepping scheme is used in this example. The two half-steps are taken after the full step.
In this example, the full step is always accepted. By tightening `e_max` to 3e-1, the reader may make
the time stepper reject the full step, and lower the time step accordingly.

!listing test/tests/time_steppers/dt2/dt2.i block=Executioner

!syntax parameters /Executioner/TimeStepper/DT2

!syntax inputs /Executioner/TimeStepper/DT2

!syntax children /Executioner/TimeStepper/DT2
