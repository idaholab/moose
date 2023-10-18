# Mortar Gap Heat Transfer Action

The mortar gap heat transfer action leverages a modular design to the computation of heat transfer. 
A mortar constraint enforcing a heat flux adds contributions of well-defined physics components. The modular contributions of
phenomena such as conduction and radiation are thus accumulated to compute an overall heat flux field. These contributions reside in user objects such as 
[GapFluxModelConduction](GapFluxModelConduction.md) and [GapFluxModelRadiation](GapFluxModelRadiation.md), which 
are timely called from `ModularGapConductanceConstraint` to build residual vectors and Jacobians.

This action can be used in two separate modes. On the one hand, the user may choose to build the physics-modeling user objects in the input. As such, the user will list
 the user objects in the `user_created_gap_flux_models` input parameter. Alternatively, the user may elect to add conduction or radiation (or both contributions simultaneously) physics 
 via the enumeration intput parameter `gap_flux_options`. 

The action can be expanded with more accurate or additional physics, which, according to this design pattern, must 
be implemented in `InterfaceUserObject`s.

!syntax parameters /MortarGapHeatTransfer

!bibtex bibliography

