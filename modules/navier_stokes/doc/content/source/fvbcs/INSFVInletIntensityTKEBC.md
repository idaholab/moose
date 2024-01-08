# INSFVInletIntensityTKEBC

This object wraps [`FVFunctionDirichletBC`](FVFunctionDirichletBC.md),
to impose a precomputed value for the turbulent kinetic energy.

The value set for the turbulent kinetic energy is:

\begin{equation}
k = \frac{3}{2} (I |\vec{u} \cdot \vec{n}|)^2 \,,
\end{equation}

where:

- $I$ is the turbulent intensity, which can be set by the user or computed via correlations, e.g., $I = 0.16 Re^{-\frac{1}{8}}$
- $\vec{u}$ is the velocity vector at a boundary face,
- $\vec{n}$ is the normal vector for a boundary face.

!syntax parameters /FVBCs/INSFVInletIntensityTKEBC

!syntax inputs /FVBCs/INSFVInletIntensityTKEBC

!syntax children /FVBCs/INSFVInletIntensityTKEBC
