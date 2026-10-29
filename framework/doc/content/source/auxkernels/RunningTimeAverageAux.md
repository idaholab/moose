# RunningTimeAverageAux

!syntax description /AuxKernels/RunningTimeAverageAux

## Description

The `RunningTimeAverageAux` computes the running time-weighted mean of a functor, which may be a
finite element or finite volume variable, a function, or a functor material property, selected
with [!param](/AuxKernels/RunningTimeAverageAux/functor). It can be used, for example, to compute
the mean field of a turbulent or otherwise unsteady simulation.

Averaging starts at the first time step with $t \geq$
[!param](/AuxKernels/RunningTimeAverageAux/start_time), and that time step is included in the
average. With $t_0$ the beginning of that first time step, the average $\bar{u}$ is updated at
each time step $n$ as

\begin{equation}
\bar{u}^n = \bar{u}^{n-1} + \left( u^n - \bar{u}^{n-1} \right) \frac{\Delta t}{t^n - t_0}
\end{equation}

where $u^n$ is the value of the functor at the current time and $\Delta t$ is the time step size.
This is equivalent to the time-weighted mean of all values since averaging started. Before
[!param](/AuxKernels/RunningTimeAverageAux/start_time) is reached, the variable holds the current
value of the functor.

The update only depends on the average from the previous time step, so executing the auxiliary
kernel several times in a time step gives the same result.

!alert note
The functor is evaluated once per element, so the auxiliary
[!param](/AuxKernels/RunningTimeAverageAux/variable) must be elemental and should be of
`CONSTANT MONOMIAL` type. Nodal variables are rejected.

## Example Input Syntax

In this example, the function `u` equals $t x$ and the simulation time advances by 1 each time
step. The `RunningTimeAverageAux` stores the time average of `u` in the `CONSTANT MONOMIAL`
variable `u_avg`, starting at $t = 2$. In each element, the average is the element centroid
coordinate times the mean of the times sampled since averaging started.

!listing test/tests/auxkernels/running_time_average_aux/running_time_average.i block=AuxKernels

!syntax parameters /AuxKernels/RunningTimeAverageAux

!syntax inputs /AuxKernels/RunningTimeAverageAux

!syntax children /AuxKernels/RunningTimeAverageAux
