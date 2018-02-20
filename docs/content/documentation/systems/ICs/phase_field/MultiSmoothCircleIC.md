<!-- MOOSE Documentation Stub: Remove this when content is added. -->

# MultiSmoothCircleIC
!syntax description /ICs/MultiSmoothCircleIC

A MultismoothCircleIC is an initial condition for creating several non-overlapping sphere shaped
features in a domain at random locations. Additionally, the bubbles can vary in radius throughout the
domain. Users also have control over the random distribution option to control the placement of the
individual bubbles. Control over the values inside and outside of the bubbles is user controllable.

The placement of the bubbles is random but if the placement of a new bubble would overlap an existing
bubble, the current bubble is not placed and an additional attempt is made to place the current bubble.
This continues until the specified number of bubbles is placed or the maximum number of tries is
exceeded.

!syntax parameters /ICs/MultiSmoothCircleIC

!syntax inputs /ICs/MultiSmoothCircleIC

!syntax children /ICs/MultiSmoothCircleIC
