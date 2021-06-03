# Fixed point iteration algorithms

MOOSE provides fixed point algorithms in all its executioners. This can be used to iterate a single
application solve to converge a parameter, for example converge the mass flow rate of a fluid simulation with a target pressure drop.
But it is more often used to tightly couple multiphysics simulations, where the `MultiApp` system is leveraged to couple
two different problems, and iterating each application, transferring information between each solve, brings the coupling to convergence.

Within one app coupling iteration, MultiApps executed on `TIMESTEP_BEGIN`, the main app and MultiApps executed on `TIMESTEP_END` are executed, in that order.
The execution order of MultiApps within one group (`TIMESTEP_BEGIN` or `TIMESTEP_END`) is undefined.
The relevant data transfers happen before and after each of the two groups of MultiApps runs.
Because the `MultiApp` system allows for wrapping another levels of MultiApps, the design enables multi-level app coupling iterations automatically.

Regardless of the fixed point algorithm used, solution vectors can be relaxed to improve the stability of the convergence.
When a `MultiApp` has its own sub-apps, MOOSE allows relaxation of the `MultiApp` solution
within the main coupling iterations and within the secondary coupling iterations, where the `MultiApp` is the main app, independently.

Relaxation, or acceleration (cf secant/Steffensen's method), is performed on variables or postprocessors. These two objects encompass
most of the data transfers that are performed when coupling several applications.

!alert note
The fixed point iteration algorithms work to converge within a time step. The previous time step solution is not modified,
The Picard, secant and Steffensen algorithm do not lag part or all of the solution vector. Lagging can still be achieved
using postprocessors, auxiliary variables, or other constructs, and transferring them at the beginning / end of a time step.

## Picard fixed point iterations

Picard iterations are the default fixed point iteration algorithm. They may be relaxed, with a relaxation factor specified for the
main application in the `Executioner` block, and a relaxation factor specified for each `MultiApp` in their respective block.

Relaxed Picard fixed point iterations may be described by:

!equation
x_{n+1} = \alpha f(x_n) + (1 - \alpha) x_n

with $x_n$ the specified variable/postprocessor, $f$ a function representing the coupled problem and $\alpha$ the relaxation factor.

Convergence of Picard iterations is expected to be linear when it converges. The Picard-Lindelhof theorem provides a set of conditions under
which convergence is guaranteed.

## Secant method

The secant method is a root finding technique which follows secant lines to find the roots of a function $f$. It is adapted here for fixed point iterations.
The secant method may be described by:

!equation
x_{n+1} = x_n - \dfrac{(f(x_n) - x_n) * (x_n - x_{n-1})}{f(x_n) - x_n - f(x_{n-1}) + x_{n-1}}

with the same conventions as above. The relaxation factor, if used, is not shown here. The secant method is easily understood for 1D problems,
where $(x_n, f(x_n) - x_n)$ are the coordinates of the points used to draw the secant, of slope $\dfrac{x_n - x_{n-1}}{(f(x_n) - x_n) - (f(x_{n-1}) - x_{n-1})}$.

Convergence of the secant method is expected to be super-linear when it converges, with an order of $\dfrac{1 + \sqrt{5}}{2}$. Some conditions
for this convergence rate is that the equations are twice differentiable in their inputs, with a fixed point multiplicity of one. Oscillatory
functions and poor initial guesses can prevent convergence.

## Steffensen's method

Steffensen's method is a root finding technique based on perturbating a solution at a given point to approximate the local derivative, such that:

!equation
g(x_{n}) = \dfrac{f(x_{n} + f(x_{n}))}{f(x_{n})}

!equation
x_{n+1} = x_n - \dfrac{f(x_n)}{g(x_{n})}

The update is then similar to Newton's method which uses the exact derivative.

Convergence of Steffensen's method is expected to be quadratic when it converges. However because it requires two evaluations of the coupled
problem before computing the next term, this method is expected to be slower than the secant method. A poor initial guesses can also prevent convergence.

!alert note
When using the secant or Steffensen's methods, only specify variables and postprocessors from either the main application or the sub-applications to be
accelerated. Specifying variables or postprocessors to be updated using the acceleration method in both applications will not provide as much
acceleration, due to the current implementation of the methods. Future work may remove this limitation.
