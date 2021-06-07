# CNSFVHLLCFluidEnergyBC

The `CNSFVHLLCFluidEnergyBC` template class implements the fluid energy flux boundary condition for
Harten-Lax-Van Leer-Contact (HLLC) given either specified boundary fluid energy and
temperature functions or a
boundary pressure function. Note that these are functions corresponding to
static boundary quantities as opposed to stagnation quantities. More details
about the different template instantiations are given below.

## CNSFVHLLCSpecifiedMassFluxAndTemperatureFluidEnergyBC

!syntax parameters /FVBCs/CNSFVHLLCSpecifiedMassFluxAndTemperatureFluidEnergyBC

!syntax inputs /FVBCs/CNSFVHLLCSpecifiedMassFluxAndTemperatureFluidEnergyBC

!syntax children /FVBCs/CNSFVHLLCSpecifiedMassFluxAndTemperatureFluidEnergyBC

## CNSFVHLLCSpecifiedPressureFluidEnergyBC

!syntax parameters /FVBCs/CNSFVHLLCSpecifiedPressureFluidEnergyBC

!syntax inputs /FVBCs/CNSFVHLLCSpecifiedPressureFluidEnergyBC

!syntax children /FVBCs/CNSFVHLLCSpecifiedPressureFluidEnergyBC
