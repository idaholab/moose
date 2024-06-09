# PWCNSFVMassAdvection

This object computes the residual and Jacobian contribution of the mass
advection term for a (weakly) compressible version of the mass continuity
equation in a porous medium, e.g. this object adds the term $\nabla\cdot
\rho\vec u_d$. We apply the divergence theorem and compute the advective flux
of mass across cell/element faces.

!syntax parameters /FVKernels/PWCNSFVMassAdvection

!syntax inputs /FVKernels/PWCNSFVMassAdvection

!syntax children /FVKernels/PWCNSFVMassAdvection

!tag name=PWCNSFVMassAdvection pairs=module:navier_stokes system:fvkernels
