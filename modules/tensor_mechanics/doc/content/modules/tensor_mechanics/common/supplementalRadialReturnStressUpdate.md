In this numerical approach, a trial stress is calculated at the start of each
simulation time increment; the trial stress calculation assumed all of the new
strain increment is elastic strain:

\begin{equation}
\sigma_{trial} = C_{ijkl} \left( \Delta \epsilon_{assumed-elastic} + \epsilon_{elastic}^{old} \right)
\end{equation}

The algorithms checks to see if the trial stress state is outside of the yield
surface, as shown in the figure to the right. If the stress state is outside of
the yield surface, the algorithm recomputes the scalar effective inelastic
strain required to return the stress state to the yield surface. This approach
is given the name Radial Return because the yield surface used is the
[von Mises yield surface](https://en.wikipedia.org/wiki/Von_Mises_yield_criterion):
in the
[devitoric stress space](https://en.wikipedia.org/wiki/Cauchy_stress_tensor#Stress_deviator_tensor),
this yield surface has the shape of a circle, and the scalar inelastic strain is
assumed to always be directed at the circle center.

### Recompute Iterations on the Effective Plastic Strain Increment

The recompute radial return materials each individually calculate, using the
[Newton Method](http://mathworld.wolfram.com/NewtonsMethod.html), the amount of
effective inelastic strain required to return the stress state to the yield
surface.

\begin{equation}
\Delta p^{(t+1)} = \Delta p^t + d \Delta p
\end{equation}

where the change in the iterative effective inelastic strain is defined as the
yield surface over the derivative of the yield surface with respect to the
inelastic strain increment.
