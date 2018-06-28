# DarcyFluxPressure

!syntax description /Kernels/DarcyFluxPressure

The Darcy flux
\begin{equation}
\mathbf{q} = - \frac{K}{\mu} \left(\nabla P - \rho \mathbf{g}\right),
\end{equation}
where $K$ is permeability, $\mu$ is fluid viscosity, $P$ is pressure, $\rho$ is fluid density,
and $\mathbf{g}$ is gravity.

This kernel should be used for the pressure variable. If gravity or density are not provided
(the default behaviour), this kernel is identical to the Diffusion kernel.

!alert note
$K/\mu$ and $\rho$ are expected as material properties called conductivity and density, respectively.

!syntax parameters /Kernels/DarcyFluxPressure

!syntax inputs /Kernels/DarcyFluxPressure

!syntax children /Kernels/DarcyFluxPressure
