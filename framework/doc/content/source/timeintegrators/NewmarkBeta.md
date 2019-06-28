# NewmarkBeta

!syntax description /Executioner/TimeIntegrator/NewmarkBeta

## Description

Newmark time integration [!citep](newmark1959amethod) is one of the commonly used time integration methods in structural dynamics problems. In this method, the second ($\ddot{u}$) and first ($\dot{u}$) time derivatives of a variable $u$ at $t+\Delta t$ are written in terms of the $u$, $\dot{u}$ and $\ddot{u}$ at time $t$, and $u$ at $t+\Delta t$ as shown below:

\begin{equation}
\begin{aligned}
\mathbf{\ddot{u}}(t+\Delta t) &=& \frac{\mathbf{u}(t+\Delta t)-\mathbf{u}(t)}{\beta \Delta t^2}- \frac{\mathbf{\dot{u}}(t)}{\beta \Delta t}+\frac{\beta -0.5}{\beta}\mathbf{\ddot{u}}(t) \\
\mathbf{\dot{u}}(t+ \Delta t) &=& \mathbf{\dot{u}}(t)+ (1-\gamma)\Delta t \mathbf{\ddot{u}}(t) + \gamma \Delta t \mathbf{\ddot{u}}(t+\Delta t)
\end{aligned}
\end{equation}

In the above equations, $\beta$ and $\gamma$ are Newmark time integration parameters.

- For $\beta = \frac{1}{4}$ and $\gamma = \frac{1}{2}$, the Newmark time integration method is implicit, unconditionally stable and second order accurate in time. This is the constant average acceleration method with no numerical damping.
- $\beta = \frac{1}{6}$ and $\gamma = \frac{1}{2}$ results in the linear acceleration method where the acceleration is linearly varying between $t$ and $t+\Delta t$. This method is also implicit, unconditionally stable and second order accurate in time. However, there is a small numerical damping when the linear acceleration method is used.
- For $\gamma = \frac{1}{2}$, the method is second order accurate and it is unconditionally stable for $\frac{1}{2} \le \gamma \le 2 \beta$.

When using the constant average acceleration method that has no numerical damping, high frequency noise can sometimes be observed in the velocity and acceleration time histories for a problem with prescribed displacement  [!citep](bathe2012insight). Using other parameters for $\beta$ and $\gamma$ results in non-zero numerical damping that damps out part of the high frequency noise but not all of it. Hilber-Hughes-Taylor (HHT) time integration is a variation of the Newmark method that damps out high frequency noise especially in structural dynamics problems. More details about this Newmark and HHT time integration schemes can be found in these [lecture notes](http://people.duke.edu/~hpgavin/cee541/NumericalIntegration.pdf). HHT time integration requires modification to the equation of motion and is currently implemented only for structural dynamics problems in tensor mechanics module.

!syntax parameters /Executioner/TimeIntegrator/NewmarkBeta

!syntax inputs /Executioner/TimeIntegrator/NewmarkBeta

!syntax children /Executioner/TimeIntegrator/NewmarkBeta

!bibtex bibliography
