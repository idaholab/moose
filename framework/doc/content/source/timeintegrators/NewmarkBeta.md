# NewmarkBeta

!syntax description /Executioner/TimeIntegrator/NewmarkBeta

## Description

Newmark time integration [citep:newmark1959amethod] is one of the commonly used time integration methods in structural dynamics problems. In this method, the second ($\ddot{u}$) and first (\dot{u}) time derivatives of a variable $u$ at $t+\Delta t$ are written in terms of the $u$, $\dot{u}$ and $\ddot{u}$ at time $t$, and $u$ at $t+\Delta t$ as shown below:

\begin{equation}
\begin{aligned}
\mathbf{\ddot{u}}(t+\Delta t) &=& \frac{\mathbf{u}(t+\Delta t)-\mathbf{u}(t)}{\beta \Delta t^2}- \frac{\mathbf{\dot{u}}(t)}{\beta \Delta t}+\frac{\beta -0.5}{\beta}\mathbf{\ddot{u}}(t) \\
\mathbf{\dot{u}}(t+ \Delta t) &=& \mathbf{\dot{u}}(t)+ (1-\gamma)\Delta t \mathbf{\ddot{u}}(t) + \gamma \Delta t \mathbf{\ddot{u}}(t+\Delta t)
\end{aligned}
\end{equation}

In the above equations, $\beta$ and $\gamma$ are Newmark time integration parameters.

- For $\beta = \frac{1}{4}$ and $\gamma = \frac{1}{2}$, the Newmark time integration method is implicit and unconditionally stable. This is the constant average acceleration method with no numerical damping. This is recommended only when a constant timestep is used throughout the simulation. If for some reason, the simulation does not converge and the timestep is halved, this time integration method with no numerical damping can result in high frequency noise.
- $\beta = \frac{1}{6}$ and $\gamma = \frac{1}{2}$ results in the linear acceleration method where the acceleration is linearly varying between $t$ and $t+\Delta t$.

More details about this time integration scheme can also be found in these [lecture notes](http://people.duke.edu/~hpgavin/cee541/NumericalIntegration.pdf).

!syntax parameters /Executioner/TimeIntegrator/NewmarkBeta

!syntax inputs /Executioner/TimeIntegrator/NewmarkBeta

!syntax children /Executioner/TimeIntegrator/NewmarkBeta

!bibtex bibliography
