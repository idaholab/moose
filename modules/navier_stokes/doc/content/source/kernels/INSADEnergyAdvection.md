# INSADEnergyAdvection

This object adds a $\rho c_p \vec u \cdot \nabla T$ term, likely to a heat
conduction-convection equation, where $\rho$ is the density, $c_p$ is the
specific heat capacity,
$\vec u$ is the velocity (represented by `_U` in the code), and $\nabla T$ is the temperature gradient
(represented by `_grad_u` in the code). The divergence form of this term is
given by

\begin{equation}
\nabla \cdot \rho c_p \vec{u} T
\end{equation}

which assuming constant $\rho$ and $c_p$ (a big assumption) can be split into
the two terms

\begin{equation}
\rho c_p\left(T \nabla \cdot \vec{u} + \vec{u} \nabla T\right)
\end{equation}

Applying the incompressibility constraint $\nabla \cdot \vec{u}$ yields the
final form used in the `INSADEnergyAdvection` object

\begin{equation}
\rho c_p \vec{u} \cdot \nabla T
\end{equation}

stated above.

!syntax description /Kernels/INSADEnergyAdvection

!syntax parameters /Kernels/INSADEnergyAdvection

!syntax inputs /Kernels/INSADEnergyAdvection

!syntax children /Kernels/INSADEnergyAdvection
