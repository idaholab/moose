# LevelSetAdvectionSUPG

This kernel adds the Streamline Upwind/Petrov-Galerkin (SUPG) stabilization
term [!citep](brooks1982streamline, donea2003finite) to the advection term of the level set equation.

\begin{equation}
\label{eq:LevelSetAdvectionSUPG:weak}
\left(-\tau \vec{v} \psi_i,\, \vec{v}\cdot\nabla u_h\right) = 0,
\end{equation}
where $\vec{v}$ is the level set velocity, $f$ is the forcing term, and $\tau$ as defined below.

\begin{equation}
\label{eq:LevelSetAdvectionSUPG:tau}
\tau = \frac{h}{2\|\vec{v}\|},
\end{equation}
where $h$ is the element length.

## Example Syntax

The LevelSetAdvectionSUPG [Kernel](syntax/Kernels/index.md) should be used in conjunction with a complete level set equation.
For example, the following provides the necessary objects for the complete level set equation
with SUPG stabilization.

!listing modules/level_set/examples/vortex/vortex_supg.i block=Kernels


!syntax parameters /Kernels/LevelSetAdvectionSUPG

!syntax inputs /Kernels/LevelSetAdvectionSUPG

!syntax children /Kernels/LevelSetAdvectionSUPG



!bibtex bibliography
