# DomainIntegralQFunction

!syntax description /AuxKernels/DomainIntegralQFunction

# Description

This object is used to compute the value of the $q$ function used in the computation of fracture domain integrals, as described in [FractureIntegrals](/FractureIntegrals.md). The $q$ function is computed based on the distance of a given point from the crack front. It evaluates to 1 if the point is within the inner radius of the integration domain, 0 if it is beyond the outer radius, and is linearly interpolated between the inner and outer radius. In 3-dimensional simulations, this function also ramps up linearly from 0 to its full value based on its position tangentially along a crack relative to the point that it is associated with along the crack front along the segments of the crack front connected to that point. 

This object is not typically directly defined by a user, but is defined using the [DomainIntegralAction](/DomainIntegralAction.md) block. Also, this object is used only for output purposes for debugging models, as the actual value of the $q$ function used in the computation is computed by the fracture domain integral Postprocessor objects.

!syntax parameters /AuxKernels/DomainIntegralQFunction

!syntax inputs /AuxKernels/DomainIntegralQFunction

!syntax children /AuxKernels/DomainIntegralQFunction
