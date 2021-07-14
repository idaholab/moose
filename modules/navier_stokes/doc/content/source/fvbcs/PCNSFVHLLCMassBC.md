# PCNSFVHLLCMassBC

The `PCNSFVHLLCMassBC` template class implements the mass flux boundary condition for
porous Harten-Lax-Van Leer-Contact (HLLC) given either specified boundary momentum and
temperature functions or a
boundary pressure function. Note that these are functions corresponding to
static boundary quantities as opposed to stagnation quantities. More details
about the different template instantiations are given below.

## PCNSFVHLLCSpecifiedMassFluxAndTemperatureMassBC

!syntax parameters /FVBCs/PCNSFVHLLCSpecifiedMassFluxAndTemperatureMassBC

!syntax inputs /FVBCs/PCNSFVHLLCSpecifiedMassFluxAndTemperatureMassBC

!syntax children /FVBCs/PCNSFVHLLCSpecifiedMassFluxAndTemperatureMassBC

## PCNSFVHLLCSpecifiedPressureMassBC

!syntax parameters /FVBCs/PCNSFVHLLCSpecifiedPressureMassBC

!syntax inputs /FVBCs/PCNSFVHLLCSpecifiedPressureMassBC

!syntax children /FVBCs/PCNSFVHLLCSpecifiedPressureMassBC
