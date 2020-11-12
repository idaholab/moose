# INSFVMassAdvectionFunctionBC

This object is only meant for use in MMS testing. It computes the residual and
Jacobian contribution of the
incompressible version of the mass continuity equation, e.g. $\nabla\cdot\vec
u$. We apply the divergence theorem and compute the advective flux of mass
across boundary faces. Second order convergence is preserved by using
exact solution information to construct ghost cell information.

!syntax parameters /FVBCs/INSFVMassAdvectionFunctionBC

!syntax inputs /FVBCs/INSFVMassAdvectionFunctionBC

!syntax children /FVBCs/INSFVMassAdvectionFunctionBC
