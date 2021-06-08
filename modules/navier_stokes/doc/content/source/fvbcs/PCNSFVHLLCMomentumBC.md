# PCNSFVHLLCMomentumBC

The `PCNSFVHLLCMomentumBC` template class implements the momentum flux boundary condition for
porous Harten-Lax-Van Leer-Contact (HLLC) given either specified boundary momentum and
temperature functions or a
boundary pressure function. Note that these are functions corresponding to
static boundary quantities as opposed to stagnation quantities. More details
about the different template instantiations are given below.

## PCNSFVHLLCSpecifiedMassFluxAndTemperatureMomentumBC

!syntax parameters /FVBCs/PCNSFVHLLCSpecifiedMassFluxAndTemperatureMomentumBC

!syntax inputs /FVBCs/PCNSFVHLLCSpecifiedMassFluxAndTemperatureMomentumBC

!syntax children /FVBCs/PCNSFVHLLCSpecifiedMassFluxAndTemperatureMomentumBC

## PCNSFVHLLCSpecifiedPressureMomentumBC

!syntax parameters /FVBCs/PCNSFVHLLCSpecifiedPressureMomentumBC

!syntax inputs /FVBCs/PCNSFVHLLCSpecifiedPressureMomentumBC

!syntax children /FVBCs/PCNSFVHLLCSpecifiedPressureMomentumBC
