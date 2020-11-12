# INSFVMomentumPressure

This object adds the $\nabla p$ term of the
incompressible Navier Stokes momentum equation. We manipulate this term to
become: $\nabla \cdot Ip$ where $I$ denotes the identity matrix. After term
manipulation we apply the divergence theorem such that the associated residual
is accumulated over faces (e.g. `FVFluxKernel`) as opposed to cell volumes (e.g.
`FVElementalKernel`). Note that the translation from divergence to gradient is
not one-to-one in coordinate systems other than Cartesian. Consequently if
running with RZ coordinates, the user must also add the
[INSFVMomentumPressureRZ.md] object in their input file.

!syntax parameters /FVKernels/INSFVMomentumPressure

!syntax inputs /FVKernels/INSFVMomentumPressure

!syntax children /FVKernels/INSFVMomentumPressure
