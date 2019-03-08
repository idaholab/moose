# INSADMomentumSUPG

This object adds stabilization to the incompressible momentum equation in
the form of $\left(\tau \vec u \cdot \nabla\psi_i, \vec R\right)$ where $\tau$
is the stabilization parameter, $\vec u$ is the velocity vector and $\vec R$ is the strong
residual of the momentum equation. This term adds additional (consistent)
streamline diffusion such that higher Reynolds numbers can be simulated without
producing crippling oscillations.

!syntax description /Kernels/INSADMomentumSUPG<RESIDUAL>

!syntax parameters /Kernels/INSADMomentumSUPG<RESIDUAL>

!syntax inputs /Kernels/INSADMomentumSUPG<RESIDUAL>

!syntax children /Kernels/INSADMomentumSUPG<RESIDUAL>
