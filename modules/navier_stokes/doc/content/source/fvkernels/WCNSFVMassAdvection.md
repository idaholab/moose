# WCNSFVMassAdvection

This object computes the residual and Jacobian contribution of the mass
advection term for a (weakly) compressible version of the mass continuity
equation, e.g. this object adds the term $\nabla\cdot \rho \vec u$. We apply the
divergence theorem and compute the advective flux of mass across cell/element
faces.

!syntax parameters /FVKernels/WCNSFVMassAdvection

!syntax inputs /FVKernels/WCNSFVMassAdvection

!syntax children /FVKernels/WCNSFVMassAdvection
