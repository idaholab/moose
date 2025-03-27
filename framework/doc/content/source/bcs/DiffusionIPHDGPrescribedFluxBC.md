# DiffusionIPHDGPrescribedFluxBC

This class weakly sets $-D \nabla u \cdot \hat{n}$ to the value provided in [!param](/BCs/DiffusionIPHDGPrescribedFluxBC/prescribed_normal_flux) which defaults to 0, the natural condition. This class is for use in a hybridized discretization of an interior penalty discontinuous Galerkin method.

!syntax parameters /BCs/DiffusionIPHDGPrescribedFluxBC

!syntax inputs /BCs/DiffusionIPHDGPrescribedFluxBC

!syntax children /BCs/DiffusionIPHDGPrescribedFluxBC
