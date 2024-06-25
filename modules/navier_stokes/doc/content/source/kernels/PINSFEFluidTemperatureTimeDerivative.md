# PINSFEFluidTemperatureTimeDerivative

!syntax description /Kernels/PINSFEFluidTemperatureTimeDerivative

If using a conservative form of the energy equation, the time derivative
of the energy will include the time derivative of the density, computed from
the fluid properties in the [!param](/Kernels/PINSFEFluidTemperatureTimeDerivative/eos)
parameter.

!alert note
The porosity is assumed to be constant in time.

!syntax parameters /Kernels/PINSFEFluidTemperatureTimeDerivative

!syntax inputs /Kernels/PINSFEFluidTemperatureTimeDerivative

!syntax children /Kernels/PINSFEFluidTemperatureTimeDerivative
