# CNSFVHLLCMassBC

The `CNSFVHLLCMassBC` template class implements the mass flux boundary condition for
Harten-Lax-Van Leer-Contact (HLLC) given either specified boundary momentum and
temperature functions or a
boundary pressure function. Note that these are function corresponding to
static boundary quantities as opposed to stagnation quantities. More details
about the different template instantiations are given below.

## CNSFVHLLCSpecifiedMassFluxAndTemperatureMassBC

!syntax parameters /FVBCs/CNSFVHLLCSpecifiedMassFluxAndTemperatureMassBC

!syntax inputs /FVBCs/CNSFVHLLCSpecifiedMassFluxAndTemperatureMassBC

!syntax children /FVBCs/CNSFVHLLCSpecifiedMassFluxAndTemperatureMassBC

## CNSFVHLLCSpecifiedPressureMassBC

!syntax parameters /FVBCs/CNSFVHLLCSpecifiedPressureMassBC

!syntax inputs /FVBCs/CNSFVHLLCSpecifiedPressureMassBC

!syntax children /FVBCs/CNSFVHLLCSpecifiedPressureMassBC
