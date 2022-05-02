# Mortar Gap Heat Transfer Action

The mortar gap heat transfer action leverages a modular design to the computation of heat transfer. 
A mortar constraint enforcing a heat flux adds contributions of well-defined physics components. The modular contributions of
phenomena such as conduction and radiation are thus accumulated to compute an overall heat flux field. These contributions reside in user objects such as 
[GapFluxModelConduction](GapFluxModelConduction.md) and [GapFluxModelRadiation](GapFluxModelRadiation.md), which 
are timely called from `ModularGapConductanceConstraint` to build residual vectors and Jacobians.

This action can be expanded with more accurate or additional physics, which, according to this design pattern, must 
be implemented in `InterfaceUserObject`s.

!syntax parameters /MortarGapHeatTransfer

!bibtex bibliography

