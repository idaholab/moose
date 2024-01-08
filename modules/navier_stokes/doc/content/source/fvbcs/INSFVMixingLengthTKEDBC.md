# INSFVMixingLengthTKEDBC

This object wraps [`FVFunctionDirichletBC`](FVFunctionDirichletBC.md),
to impose a precomputed value for the turbulent kinetic energy dissipation rate.

The value set for the turbulent kinetic energy is:

\begin{equation}
\epsilon = C_{\mu}^{0.75} \frac{k^{\frac{3}{2}}}{0.07 L} \,,
\end{equation}

where:

- $C_{\mu} = 0.09$ is a closure parameter,
- $k$ is the turbulent kinetic energy,
- $L$ is a characteristic length, e.g., the diameter of a pipe, diameter of an inlet jet, or the side length of the lid-driven cavity.

!syntax parameters /FVBCs/INSFVMixingLengthTKEDBC

!syntax inputs /FVBCs/INSFVMixingLengthTKEDBC

!syntax children /FVBCs/INSFVMixingLengthTKEDBC
