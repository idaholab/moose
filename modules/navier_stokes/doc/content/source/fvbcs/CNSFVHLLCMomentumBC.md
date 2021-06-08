# CNSFVHLLCMomentumBC

The `CNSFVHLLCMomentumBC` template class implements the momentum flux boundary condition for
Harten-Lax-Van Leer-Contact (HLLC) given either specified boundary momentum and
temperature functions or a
boundary pressure function. Note that these are functions corresponding to
static boundary quantities as opposed to stagnation quantities. More details
about the different template instantiations are given below.

## CNSFVHLLCSpecifiedMassFluxAndTemperatureMomentumBC

!syntax parameters /FVBCs/CNSFVHLLCSpecifiedMassFluxAndTemperatureMomentumBC

!syntax inputs /FVBCs/CNSFVHLLCSpecifiedMassFluxAndTemperatureMomentumBC

!syntax children /FVBCs/CNSFVHLLCSpecifiedMassFluxAndTemperatureMomentumBC

## CNSFVHLLCSpecifiedPressureMomentumBC

!syntax parameters /FVBCs/CNSFVHLLCSpecifiedPressureMomentumBC

!syntax inputs /FVBCs/CNSFVHLLCSpecifiedPressureMomentumBC

!syntax children /FVBCs/CNSFVHLLCSpecifiedPressureMomentumBC
