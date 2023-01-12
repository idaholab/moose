# ADNumericalFlux3EqnHLLC

!syntax description /UserObjects/ADNumericalFlux3EqnHLLC

This class implements the HLLC Riemann solver for the 1-D, variable-area Euler
equations, which computes a numerical flux from two solution values:
\begin{equation}
  \mathbf{F} = \mathcal{F}(\mathbf{U}_1, \mathbf{U}_2, \mathbf{n}) \,,
\end{equation}
where $\mathbf{n}$ is the normal unit vector in the direction of cell 2 from
cell 1. This implementation is based on the work by Batten et al.
[!cite](batten1997average), but the equation of state is generalized instead of
assuming ideal gas.

!bibtex bibliography

!syntax parameters /UserObjects/ADNumericalFlux3EqnHLLC

!syntax inputs /UserObjects/ADNumericalFlux3EqnHLLC

!syntax children /UserObjects/ADNumericalFlux3EqnHLLC
