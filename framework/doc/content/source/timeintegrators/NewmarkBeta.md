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

When using Newmark time integration in structural dynamics problems that require an initial static step (most commonly for gravity analysis), a convenient method in MOOSE is to disable the inertia kernels (which can be done using the [controls system](syntax/Controls/index.md)), the velocity and acceleration calculations, and the stiffness damping (which can be done by setting `static_initialization=true` in the stressdivergence kernels) during the first time step. This leads to solving the equation, Ku = F, in the first time step, which essentially initializes displacements and stresses from gravity loading. When using the Newmark-Beta time integrator (which we most often use for dynamics) or any other time integrator, we cannot switch off time derivatives (velocity and acceleration) calculations through the control system. Therefore, the time integrator will compute velocity and acceleration for the static step. When using the Newmark-Beta time integration for this purpose, this will result in noisy acceleration and velocity responses in the whole simulation. Such spurious responses can be avoided by using the `inactive_tsteps` parameter. This parameter ensures that the the NewmarkBeta time integrator returns zero derivatives for the first few time steps and starting the acceleration and velocity calculations after that. The time derivative calculations are started when the time step number is greater than `inactive_tsteps`.

A sample result of using this parameter is shown in [fig:disp], [fig:vel], and [fig:accel] below. The result corresponds to the time derivative of a ramp function, which is typically the displacement response under gravity. The velocities and accelerations calculated for this function without using the `inactive_tsteps` parameter (blue) and using `inactive_tsteps=1` (orange) are shown. The input syntax used to generate the orange plots below is listed after the figures below.

!row!

!media time_integrators/inactive_tsteps_u.png
       style=width:30%;float:left;padding-top:2.5%
       id=fig:disp
       caption=Displacement

!media time_integrators/inactive_tsteps_udot.png
      style=width:30%;float:left;padding-top:2.5%
      id=fig:vel
      caption=Velocity

!media time_integrators/inactive_tsteps_udotdot.png
       style=width:30%;float:left;padding-top:2.5%
       id=fig:accel
       caption=Acceleration

!row-end!

!listing test/tests/time_integrators/newmark-beta/newmark_beta_inactive_steps.i block=Executioner

!syntax parameters /Executioner/TimeIntegrator/NewmarkBeta

!syntax inputs /Executioner/TimeIntegrator/NewmarkBeta

!syntax children /Executioner/TimeIntegrator/NewmarkBeta

!bibtex bibliography
