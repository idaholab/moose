# PCNSFVHLLCFluidEnergyBC

The `PCNSFVHLLCFluidEnergyBC` template class implements the momentum flux boundary condition for
porous Harten-Lax-Van Leer-Contact (HLLC) given either specified boundary momentum and
temperature functions or a
boundary pressure function. Note that these are functions corresponding to
static boundary quantities as opposed to stagnation quantities. More details
about the different template instantiations are given below.

## PCNSFVHLLCSpecifiedMassFluxAndTemperatureFluidEnergyBC

!syntax parameters /FVBCs/PCNSFVHLLCSpecifiedMassFluxAndTemperatureFluidEnergyBC

!syntax inputs /FVBCs/PCNSFVHLLCSpecifiedMassFluxAndTemperatureFluidEnergyBC

!syntax children /FVBCs/PCNSFVHLLCSpecifiedMassFluxAndTemperatureFluidEnergyBC

## PCNSFVHLLCSpecifiedPressureFluidEnergyBC

!syntax parameters /FVBCs/PCNSFVHLLCSpecifiedPressureFluidEnergyBC

!syntax inputs /FVBCs/PCNSFVHLLCSpecifiedPressureFluidEnergyBC

!syntax children /FVBCs/PCNSFVHLLCSpecifiedPressureFluidEnergyBC
