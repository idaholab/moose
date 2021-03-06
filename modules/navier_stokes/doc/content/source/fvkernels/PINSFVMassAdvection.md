# PINSFVMassAdvection

This object computes the residual and Jacobian contribution of the
incompressible version of the mass continuity equation, e.g. $\nabla\cdot \rho \vec
u_d$. We apply the divergence theorem and compute the advective flux of mass
across cell/element faces.

When using the Rhie-Chow interpolation to compute the mass flow across faces, this kernel
reverts by default to average interpolation near porosity changes. This behavior may be disabled
with the `smooth_porosity` boolean.

!syntax parameters /FVKernels/PINSFVMassAdvection

!syntax inputs /FVKernels/PINSFVMassAdvection

!syntax children /FVKernels/PINSFVMassAdvection
