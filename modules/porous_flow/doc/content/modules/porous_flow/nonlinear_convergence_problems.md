# Nonlinear convergence problems

Even if the [convergence criteria](/porous_flow/convergence.md) are set appropriately, MOOSE sometimes finds it very difficult to reduce the nonlinear residual.  Some analysis can usually fix this, although sometimes it's just too bad: your simulation is just very nonlinear.  Here are some tips for analysing and fixing poor nonlinear convergence.

## Fluid equations of state

These can be highly nonlinear and take a long time to evaluate.  When diagnosing convergence problems, it is always worthwhile replacing your equations of state with appropriate `SimpleFluidProperties` because it is a very basic change to the input file.

If MOOSE keeps sampling outside the region of validity of your equation of state (and thereby causing a timestep reduction) and if you know the final result should be within the region of validity, use `TabulatedFluidProperties` instead.  They are much faster to evaluate too!

## Boundary conditions

Inspect your boundary conditions very closely.  If variable $v$ is associated to the mass-balance equation for component $c$, then a preset `DirichletBC` for `variable = v` is physically saying "add or remove component $c$ in order to keep variable $v$ fixed".  Is this really what you want?  What if you're trying to remove component $c$ but there is no component $c$ at the node in question?

Almost always it is better to use a [`PorousFlowSink`](PorousFlowSink.md) (see also [boundaries](boundaries.md)) instead.  This is numerically "smoother" than a preset `DirichletBC` and also may be more easily interpreted physically as the influence of a boundary situated at $\infty$.

## External fluxes

External fluxes that turn off or on too quickly are bad.  Similarly,
  those that have large discontinuities in their derivatives can cause
  convergence problems.  Try to define "smooth" versions of these as
  inputs.  Discontinuities like these often manifest themselves in
  non-convergence of the nonlinear iterations.

## Capillarity and effective saturation

Effective saturation curves that are too "flat" are not good.
  For example, the van Genuchten parameter $m=0.6$ almost always gives
  better convergence than $m=0.9$.

Effective saturation curves that are too "low" are not good.
  For example, the van Genuchten parameter $\alpha=10^{-6}\,$Pa$^{-1}$
  almost always gives better convergence than
  $\alpha=10^{-3}\,$Pa$^{-1}$.

Any discontinuities in the effective saturation, or its
  derivative, are bad.  I suggest using van Genuchten parameter
  $\alpha\geq 0.5$ for problems with both saturated and unsaturated
  zones if the van Genuchten relative permeability relation is used.

The van Genuchten capillarity relationships have an `s_scale` parameter that may sometimes be used in 2-phase simulations to great effect.

## Relative permeability relationships

Highly nonlinear relative permeability curves make convergence
  difficult in some cases.  For instance, a Corey relative
  permeability curve with $n=20$ is much worse numerically than with
  $n=2$.  See if you can reduce the nonlinearity in your curve.

## Multiphase problems

In multiphase problems if one phase completely disappears, MOOSE
  may not converge.  If one phase almost disappears, MOOSE may take an exorbitant number of iterations to converge.  To avoid this:

 - The fully-upwind kernel and boundary fluxes and dirac sources
  can be used.  If the residual saturation of the phase is nonzero,
  then it probably won't disappear, as the fully-upwind approach will
  not, in theory, allow fluid to exit from a node if the relative
  permeability is zero.  However, numerical imprecision can lead to
  phase disappearance.
 - A nonzero residual saturation can be used.  This means that for
  $dt\rightarrow 0$ the Jacobian matrix will typicaly be nonsingular.  Then in most cases the problematic node will
  fill with a little amount of the phase in the next time step.
 - A "shifted" van Genuchten capillary suction curve may be used (using the `s_scale` parameter)
  in difficult multiphase problems.

## More details containing phase disappearance

Consider a 2-phase simulation containing just a single node (with no
Darcy flux) and with the mass fractions fixed at 0 or 1.  The residual
is just
\begin{equation}
R = \frac{d}{dt} \left(
\begin{array}{c}
\phi\rho_{g}S_{g} \\
\phi\rho_{w}S_{w}
\end{array}
\right)
\end{equation}
Here the phases have been labelled by "g" and "w".  With the variables being $P_{g}$ and $P_{w}$ the Jacobian is

\begin{equation}
J = \frac{\phi}{dt} \left(
\begin{array}{cc}
\rho_{g}'S_{g} + \rho_{g}\frac{\partial S_{g}}{\partial P_{g}} &
\rho_{g}\frac{\partial S_{g}}{\partial P_{w}} \\
\rho_{w}\frac{\partial S_{w}}{\partial P_{g}} &
\rho_{w}'S_{w} + \rho_{w}\frac{\partial S_{w}}{\partial P_{w}}
\end{array}
\right)
= \frac{\phi}{dt} \left(
\begin{array}{cc}
\rho_{g}'S_{g} + \rho_{g}S' &
-\rho_{g}S' \\
-\rho_{w}S' &
\rho_{w}'S_{w} + \rho_{w}S'
\end{array}
\right)
\end{equation}
However, as $S_{w}\rightarrow 1$, the standard van Genuchten expression for $S'$ gives $S'=0$, so
\begin{equation}
J = \frac{\phi}{dt} \left(
\begin{array}{cc}
0 & 0 \\
0 &
\rho_{w}'
\end{array}
\right)
\end{equation}
which is singular!  This singularity usually manifests in one of two ways:

 - PETSc finds it impossible to invert the Jacobian
 - The nonlinear solver finds it difficult or impossible to converge

Using the `s_scale` parameter means that $S'>0$ for all finite porepressures, so the determinant is always positive
\begin{equation}
\det J = \rho_{g}'\rho_{w}'S(1-S) + \rho_{g}'\rho_{w}SS' +
\rho_{g}\rho_{w}'(1-S)S' > 0 \ .
\end{equation}
since $0\leq S\leq 1$, and $\rho'>0$ physically, so the Jacobian is
non-singular.

## Multi-phase situations using other variables

PorousFlow allows the use of other variables, not just porepressures, but mixtures of porepressures and saturations.  There is no free lunch, however, and when nonlinear problems start to manifest themselves, a thorough analysis such as the one presented above will identify the cause of the problems.

## Component disappearance

Similar remarks may be made about component disappearance.  Often times it is useful to explicitly write the equations to determine where the zeroes (and noninvertability) are occuring.
