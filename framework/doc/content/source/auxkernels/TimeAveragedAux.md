# TimeAveragedAux

!syntax description /AuxKernels/TimeAveragedAux

## Description

The `TimeAveragedAux` computes the running time-weighted mean of a functor, which may be a
finite element or finite volume variable, a function, or a functor material property, selected
with [!param](/AuxKernels/TimeAveragedAux/functor). It can be used, for example, to compute
the mean field of a turbulent or otherwise unsteady simulation.

Averaging starts at the first time step with $t \geq$
[!param](/AuxKernels/TimeAveragedAux/start_time), and that time step is included in the
average. With $t_0$ the beginning of that first time step, the average $\bar{u}$ is updated at
each time step $n$ as

\begin{equation}
\bar{u}^n = \bar{u}^{n-1} + \left( u^n - \bar{u}^{n-1} \right) \frac{\Delta t}{t^n - t_0}
\end{equation}

where $u^n$ is the value of the functor at the current time and $\Delta t$ is the time step size.
This is equivalent to the time-weighted mean of all values since averaging started. Before
[!param](/AuxKernels/TimeAveragedAux/start_time) is reached, the variable holds the current
value of the functor.

By default, the functor is integrated over each time step with the rectangle rule using its
current value $u^n$, the same quadrature as the implicit Euler time integrator. This is first
order accurate in $\Delta t$, and for statistically stationary signals averaged over many time
steps the quadrature error is much smaller than the sampling error of the average. For more
accuracy, setting [!param](/AuxKernels/TimeAveragedAux/time_integration_scheme) to
`trapezoidal-rule` replaces $u^n$ with the trapezoidal rule over the time step,
$\left( u^n + u^{n-1} \right) / 2$, which is second order accurate. The `time-integrator` option
uses the trapezoidal rule when every time integrator of the executioner is at least second
order, for example [CrankNicolson.md] or [BDF2.md], and the rectangle rule otherwise. The
trapezoidal rule requires the functor to support evaluation at the previous time step.

The update only depends on the average from the previous time step, so executing the auxiliary
kernel several times in a time step gives the same result.

!alert note
The functor is evaluated once per element, so the auxiliary
[!param](/AuxKernels/TimeAveragedAux/variable) must be elemental and should be of
`CONSTANT MONOMIAL` type. Nodal variables are rejected.

## Example Input Syntax

In this example, the function `u` equals $t x$ and the simulation time advances by 1 each time
step. The `TimeAveragedAux` stores the time average of `u` in the `CONSTANT MONOMIAL`
variable `u_avg`, starting at $t = 2$. In each element, the average is the element centroid
coordinate times the mean of the times sampled since averaging started.

!listing test/tests/auxkernels/time_averaged_aux/time_averaged.i block=AuxKernels

!syntax parameters /AuxKernels/TimeAveragedAux

!syntax inputs /AuxKernels/TimeAveragedAux

!syntax children /AuxKernels/TimeAveragedAux
