# AdvectionIPHDGPrescribedFluxBC

This class is the advection analog of [DiffusionIPHDGPrescribedFluxBC.md]. It is used to prescribe an advective flux for a hybridized interior penalty discontinuous Galerkin discretization of an advection term. If the prescribed flux is 0, it is equivalent to prescribing a zero advective flow condition, which is appropriate for wall-type boundaries. This class may also be used to prescribe an inlet flux condition; in that case the prescribed flux should be negative, denoting an inflow. This class should not be used at outflow boundaries because the advective flux at such boundaries should be computed implicitly.

!syntax parameters /BCs/AdvectionIPHDGPrescribedFluxBC

!syntax inputs /BCs/AdvectionIPHDGPrescribedFluxBC

!syntax children /BCs/AdvectionIPHDGPrescribedFluxBC
