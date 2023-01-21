# ADNumericalFlux3EqnCentered

!syntax description /UserObjects/ADNumericalFlux3EqnCentered

This class implements a centered numerical flux for the 1-D, 1-phase, variable-area Euler
equations:
\begin{equation}
  \mathcal{F}(\mathbf{U}_1, \mathbf{U}_2, \mathbf{n})
    = \frac{1}{2} \left(\mathbf{F}(\mathbf{U}_1) + \mathbf{F}(\mathbf{U}_2)\right) \,.
\end{equation}
This flux is known to not be linearly stable, but it can be useful in some
cases, such as debugging.

!syntax parameters /UserObjects/ADNumericalFlux3EqnCentered

!syntax inputs /UserObjects/ADNumericalFlux3EqnCentered

!syntax children /UserObjects/ADNumericalFlux3EqnCentered
