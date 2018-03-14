# DomainIntegralTopologicalQFunction

!syntax description /AuxKernels/DomainIntegralTopologicalQFunction

# Description

This object is used to compute the value of the $q$ function used in the computation of fracture domain integrals, as described in [FractureIntegrals](/FractureIntegrals.md). This form of the $q$ function is based on the mesh topology. Rings of elements in layers expanding from the crack front are formed, and the $q$ function varies from 1 to 0 from the inside nodes to the outside nodes of a given ring.

This object is not typically directly defined by a user, but is defined using the [DomainIntegralAction](/DomainIntegralAction.md) block. Also, this object is used only for output purposes for debugging models, as the actual value of the $q$ function used in the computation is computed by the fracture domain integral Postprocessor objects.

!syntax parameters /AuxKernels/DomainIntegralTopologicalQFunction

!syntax inputs /AuxKernels/DomainIntegralTopologicalQFunction

!syntax children /AuxKernels/DomainIntegralTopologicalQFunction
