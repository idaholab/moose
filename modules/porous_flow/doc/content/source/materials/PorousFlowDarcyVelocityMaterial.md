# PorousFlowDarcyVelocityMaterial

!syntax description /Materials/PorousFlowDarcyVelocityMaterial

This `Material` computes the Darcy velocity of all phases in the PorousFlow system.  The Material properties are evaluated at the quadpoints (`at_nodes = false`).  The Darcy velocity of a phase $\beta$ is:
\begin{equation}
\mathbf{v}_{\beta} = -\frac{k k_{\mathrm{r,}\ \beta}}{\mu_{\beta}}(\nabla P_{\beta} - \rho_{\beta}\mathbf{g}) \ ,
\end{equation}
where $k$ is the permeability tensor, $k_{\mathrm{r,}\ \beta}$ is the relative permeability, $\mu$ is the viscosity, $P$ is the porepressure, $\rho$ is the density and $\mathbf{g}$ is the gravitational acceleration.
Hence, this `Material` requires many other `Materials`: permeability, relative permeabilities, etc.

An example of the syntax is:

!listing modules/porous_flow/test/tests/basic_advection/1phase.i start=[darcy_velocity] end=[]

The declaration of the Material name is:

!listing modules/porous_flow/src/materials/PorousFlowDarcyVelocityMaterial.C start=_darcy_velocity end=_ddarcy_velocity

This `Material` also computes the derivatives of the Darcy velocities with respect to all the PorousFlow variables, for use by Kernels, etc, to construct the Jacobian.




!syntax parameters /Materials/PorousFlowDarcyVelocityMaterial

!syntax inputs /Materials/PorousFlowDarcyVelocityMaterial

!syntax children /Materials/PorousFlowDarcyVelocityMaterial
