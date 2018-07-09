# PrimaryConvection

!syntax description /Kernels/PrimaryConvection

Convective flux of concentration of the $j^{\mathrm{th}}$ primary species.
Implements the weak form of
\begin{equation}
\mathbf{q} \cdot \left(\nabla C_j \right)
\end{equation}
where $\mathbf{q}$ is the Darcy velocity
\begin{equation}
\mathbf{q} = - \frac{K}{\mu} \left(\nabla P - \rho \mathbf{g}\right),
\end{equation}
where $K$ is the permeability tensor, $\mu$ is fluid viscosity, $P$ is pressure,
$\rho$ is fluid density, and $\mathbf{g}$ is gravity (by default, the gravity term
is not included).

!alert note
$K/\mu$ and $\rho$ are expected as material properties called conductivity and density, respectively.

!syntax parameters /Kernels/PrimaryConvection

!syntax inputs /Kernels/PrimaryConvection

!syntax children /Kernels/PrimaryConvection
