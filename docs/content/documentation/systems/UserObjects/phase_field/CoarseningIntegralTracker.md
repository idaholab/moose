# CoarseningIntegralTracker
!description /UserObjects/CoarseningIntegralTracker

This user object computes the integral of a given variable over each element at
the end of a timestep and stores it. As soon as the mesh changes the UO loops
over all elements that have undercone coarsening and looks at the difference between
the integral of the variable over the newly coarsened element and the sum of the
previously computed integrals of the refined subelements.

As coarsening drops nodes in regions with high curvature the integrals over variables
can be non-conserved. Applying the difference computed by this UO as a source term
using the [CoarseningIntegralCompensation](/CoarseningIntegralCompensation.md)
kernel will restore conservation of the integral.

!parameters /UserObjects/CoarseningIntegralTracker

!inputfiles /UserObjects/CoarseningIntegralTracker

!childobjects /UserObjects/CoarseningIntegralTracker
