# Convergence criteria

It is very important to set the global convergence criterion appropriately. This
is the `-nl_abs_tol`, or equivalently, PETSc's `-snes_atol`.  Of course the
arguments below will also inform the setting of the relative tolerance
`nl_rel_tol` (or PETSc's `-snes_rtol`).  If set too small, then MOOSE will
appear not to converge and will need very small timesteps in order to *do
anything*.  If set too large, then MOOSE will converge to the *wrong* result.
Here are some tips to estimate the global convergence criterion.

## Fluids

Determine an appopriate tolerance on what you mean by *steadystate*.  For
instance, in a single-phase simulation with reasonably large constant fluid bulk
modulus, and gravity acting in the $-z$ direction, the steadystate solution is
$P = -\rho_{0}gz$ (up to a constant).  In the case of water, this reads
$P=-10000z$ (approximately). Instead of this, suppose you would be happy to say
the model is at steadystate if $P = -(\rho_{0} g + \epsilon)z$.  Here $\epsilon$
needn't be constant: it may switch in sign, but its magnitude $|\epsilon|$ is
roughly the same over the domain of interest.  For instance, for water, an error
of  $|\epsilon|=1$ Pa.m$^{-1}$ might be suitable in your problem.

Then recall that the residual is just
\begin{equation}
\label{eq:res_int}
R = \left|\int
\nabla_{i}\left(\frac{\kappa_{ij}\kappa_{\mathrm{rel}}\rho}{\mu}(\nabla_{j}P +
\rho g_{j}) \right) \right|
\end{equation}
Evaluate this for
your *almost-steadystate* solution.

For instance, in the case of water just quoted, $\nabla P + \rho g \sim
|\epsilon|$, and the $\rho\sim\rho_{0}$, and the integral just becomes $V$, the
volume of the region of interest, so
\begin{equation}
R \approx
V|\kappa|\rho_{0}\epsilon/\mu \ .
\end{equation}

For water, this gives $R \approx V|\kappa|\times 10^{6}\epsilon$, using standard
values for $\rho_{0}$ and $\mu$.

In the previous step, an appropriate tolernace on the residual was given as
$V|\kappa|\rho_{0}\epsilon/\mu$.  Now the volume of interest, $V$, must be
specified.  Often this is not the entire mesh, but a small region where most of
the interesting dynamics occurs, and the remainder of the mesh exists just to
provide reasonable boundary conditions for this *interesting* region.  The
residual in the *boring* region can be thought of as virtually zero, while the
residual in the *interesting* region is
$V_{\mathrm{interesting}}|\kappa|\rho_{0}\epsilon/\mu$.  This is the absolute
nonlinear residual that you should aim for.

In the above, it is implicitly assumed $\kappa$ is constant, $\rho$ is virtually
constant at $\rho_{0}$, only a single-phase is present, etc.  In many cases
these assumptions are not valid, so the integral of [eq:res_int]
cannot be done as trivially as in the previous steps. In these cases, a good
approximation of a reasonable residual tolerance can be obtained by building a
model with initial conditions like $P = -(\rho_{0} g + \epsilon)z$ and observing
what the initial residual is.

## Heat

A similar method can be used to estimate the convergence criterion for heat-flow
problems.  Suppose there is both heat conduction, and fluid advection.  Suppose
that $\epsilon_{\mathrm{fluid}}$ is the acceptable error in the fluid equations
(as in the previous section).  Suppose that $\nabla T \approx
\epsilon_{\mathrm{heat}}$ is an acceptable error for the temperature.  For
instance being incorrect by $\epsilon = 10^{-3}\,$K.m$^{-1}$ might be
appropriate for your problem.

Then the residual is approximately
\begin{equation}
R \approx
V(\lambda\epsilon_{\mathrm{heat}} +
h|\kappa|\rho_{0}\epsilon_{\mathrm{fluid}}/\mu).
\end{equation}
The last term is
just the fluid enthalpy, $h$, multiplied by the fluid residual.

## Mechanics

Very similar methods can be used in simulations where mechanical deformations
are active.  Roughly speaking, MOOSE is attempting to set
$\nabla\sigma^{\mathrm{tot}} = 0$.  Determine the error in $\nabla\sigma$ that
you are willing to accept, and label it $|\epsilon|$.  For instance $\epsilon =
1$ Pa.m$^{-1}$ might be appropriate for your problem.  You may determine
$\epsilon$ from consideration of stresses directly, or you may wish to consider
what the accpetable error in strains or displacements would be, and then use the
elasticity tensor to find $\epsilon$.

The nonlinear residual will be
\begin{equation}
R \approx V|\epsilon| \ ,
\end{equation}
where $V$ is the volume of interest.

## Scaling the variables

Often it is appropriate to scale the variables in order to weight their
contributions to the overall nonlinear residual appropriately. For instance,
suppose the previous arguments provided
\begin{equation}
\begin{aligned}
R_{\mathrm{fluid}} & \sim 10^{-9}V \ , \\
R_{\mathrm{heat}} & \sim (10^{-3} + 10^{6}\times 10^{-9}) V \ , \\
R_{\mathrm{mech}} & \sim V \ ,
\end{aligned}
\end{equation}
(with the same $V$ in each case).  Then, a scaling of around
$10^9$ on the porepressure variable (or whatever MOOSE variable is associated to
the fluid equation) would be appropriate.  Similarly, a scaling of around $10^3$
on the temperature variable would be appropriate.

Scaling the variables is implemented in the input file using the `scaling` parameter, for instance:

!listing modules/porous_flow/examples/tutorial/03.i start=[Variables] end=[PorousFlowBasicTHM]
