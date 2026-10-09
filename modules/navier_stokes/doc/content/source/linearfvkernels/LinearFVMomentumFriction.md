# LinearFVMomentumFriction

This kernel adds Darcy and Forchheimer resistance to a linear finite volume momentum equation.
For superficial velocity $\mathbf{U}$, its componentwise resistance is

\begin{equation}
R_i = \left(\mu D_i + \frac{\rho}{2}F_i
\left|\frac{\mathbf{U}}{\epsilon}\right|\right)U_i,
\end{equation}

where $D_i$ and $F_i$ are the componentwise Darcy and Forchheimer coefficients. Either term may be
omitted. [!param](/LinearFVKernels/LinearFVMomentumFriction/mu) is required with
[!param](/LinearFVKernels/LinearFVMomentumFriction/Darcy_name), while density and all velocity
components present in the mesh dimension are required with
[!param](/LinearFVKernels/LinearFVMomentumFriction/Forchheimer_name).

The Forchheimer speed is evaluated from interstitial velocity $\mathbf{U}/\epsilon$ and lagged in
the linear solve. Porosity must be positive wherever the Forchheimer term is active.

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/linear-segregated/1d-simple-channel/porous-baffle-1d.i block=LinearFVKernels/u_friction

!syntax parameters /LinearFVKernels/LinearFVMomentumFriction

!syntax inputs /LinearFVKernels/LinearFVMomentumFriction

!syntax children /LinearFVKernels/LinearFVMomentumFriction
