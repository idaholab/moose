# AdvectionIPHDGOutflowBC

This class imposes an outflow advective flux for a hybridized interior penalty discontinuous Galerkin discretization. The facet solution is weakly set to the outflowing interior solution in this case to prevent the system of equations from being singular; the facet unknowns play no role in the interior solution.

!alert warning
This kernel checks, only in development/debug mode, that the velocity is indeed pointing out of the domain producing outflow conditions.

!syntax parameters /BCs/AdvectionIPHDGOutflowBC

!syntax inputs /BCs/AdvectionIPHDGOutflowBC

!syntax children /BCs/AdvectionIPHDGOutflowBC
