# MultiApp coupling algorithms

MOOSE provides MultiApp coupling algorithms in all its executioners for tightly-coupled multiphysics simulations.
MultiApps of two groups of before and after master app and master app are solved sequentially within one app coupling iteration.
The execution order of MultiApps within one group is undefined.
Relevant data transfers happen before and after each of the two groups of MultiApps runs.
Because MultiApp allows wrapping another levels of MultiApps, the design enables multi-level app coupling iterations automatically.

Regardless of the coupling algorithm used, MultiApp coupling iterations can be relaxed to improve the stability of the convergence.
When a MultiApp is a subapp of a master and a master of its own subapps, MOOSE allows relaxation of the MultiApp solution
within the master coupling iterations and within the coupling iterations, where the MultiApp is the master, independently.

Relaxation, or acceleration (cf secant/Steffensen's method), is performed on variables or postprocessors. These two objects encompass
most of the data transfers that are performed when coupling several applications.

## Picard fixed point iterations

Picard iterations are the default coupling algorithm between MultiApps. They may be relaxed, with a relaxation factor specified for the
Master application in the `Executioner` block, and a relaxation factor specified for each `MultiApp` in their respective block.

Relaxed Picard fixed point iterations may be described by:
\begin{equation}
x_{n+1} = \alpha f(x_n) + (1 - \alpha) x_n
\end{equation}
with $x_n$ the specified variable/postprocessor, $f$ a function representing the coupled problem and $\alpha$ the relaxation factor.

Convergence of Picard iterations is expected to be linear when it converges. The Picard-Lindelhof theorem provides a set of conditions under
which convergence is guaranteed.

## Secant method

The secant method is a root finding technique which follows secant lines to find the roots of f. It is adapted here for fixed point iterations.
The secant method may be described by:
\begin{eqnarray}
y_{n} = f(x_{n})
x_{n+1} = x_n - \dfrac{(f(x_n) - x_n) * (x_n - x_{n-1})}{f(x_n) - x_n - f(x_{n-1}) + x_{n-1}}
\end{eqnarray}

Convergence of the secant method is expected to be super-linear when it converges, with an order of $\dfrac{1 + \sqrt{5}}{2}. Some conditions
for this convergence rate is that the equations are twice differentiable in their inputs, with a fixed point multiplicity of one. Oscillatory
functions and poor initial guesses can prevent convergence.

## Steffensen's method

Steffensen's method is a root finding technique based on perturbating a solution at a given point to approximate the local derivative,. The update is
then similar to Newton's method which uses the exact derivative.

\begin{eqnarray}
g(x_{n}) = \dfrac{f(x_{n} + f(x_{n}))}{f(x_{n})}
x_{n+1} = x_n - \dfrac{f(x_n)}{g(x_{n})}
\end{eqnarray}

Convergence of Steffensen's method is expected to be quadratic when it converges. However because it requires two evaluations of the coupled
problem before computing the next term, this method is expected to be slower than the secant method. A poor initial guesses can also prevent convergence.

!alert note
When using the secant or Steffensen's methods, only specify variables and postprocessors from either the master application or the sub-applications to be accelerated. Specifying in both applications will not provide as much acceleration, due to the current implementation of the methods. Future work may remove this limitation.
