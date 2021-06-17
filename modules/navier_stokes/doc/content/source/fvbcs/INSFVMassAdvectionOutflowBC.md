# INSFVMassAdvectionOutflowBC

This object computes the residual and Jacobian contribution of the
incompressible version of the mass continuity equation, e.g. $\nabla\cdot\vec
u = 0$ along the domain boundary. This boundary condition should be used when it is
desired that the pressure value be extrapolated at the boundary face. If the
user wishes to have a certain value for pressure at the outflow boundary, then
they should use [INSFVOutletPressureBC](/INSFVOutletPressureBC.md).

!syntax parameters /FVBCs/INSFVMassAdvectionOutflowBC

!syntax inputs /FVBCs/INSFVMassAdvectionOutflowBC

!syntax children /FVBCs/INSFVMassAdvectionOutflowBC
