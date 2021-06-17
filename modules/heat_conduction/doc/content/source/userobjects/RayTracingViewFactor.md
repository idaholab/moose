# RayTracingViewFactor

!syntax description /UserObjects/RayTracingViewFactor

## Description

`RayTracingViewFactor` uses the ray tracing module to compute view factors for general cavities.
`RayTracingViewFactor` does not require the faces of the cavity to be planar and it allows obstruction.
Obstruction refers to the situation of boundary `a`'s line of sight to boundary `b` being obstructed
by boundary `c`, where `a`, `b`, and `c` are boundaries participating in the radiative heat exchange.
`RayTracingViewFactor` follows the general concept of radiative heat transfer geometries in MOOSE.
A set of boundaries is in radiative heat transfer through a cavity filled with a transparent medium;
the boundaries completely enclose the cavity so energy cannot leave the cavity by radiation.

For MOOSE's raytracing to work, the cavity +must+ be meshed. The view factors are computed by essentially
computing the integrals in [UnobstructedPlanarViewFactor.md]. The difference
from [UnobstructedPlanarViewFactor.md] is that the raytracer follows the
ray drawn from starting to ending quadrature points and determines if the ray collides with any of the boundaries
participating in the radiative heat transfer before it reaches the ending quadrature point. The view factor integrals
are only incremented by the contribution associated with the starting/ending quadrature point if no collision with any boundary
occurred.

The `RayTracingViewFactor` requires a [ViewFactorRayStudy.md] for computing the view factors. The user should
read the corresponding documentation.

## Important Convention on Boundaries and Normals

Boundaries (or synonymously sidesets) are a collection of element faces. An element face in MOOSE is identified
by the element id and a *local* face id. As a consequence, a face belongs to a particular element because it's
simply one of the element's faces. For element faces on the domain boundary it is clear which element they belong to
because there is no other element across the face. However, for internal faces the question which of the two neighboring
elements an element face belongs to is important. An example helps to clarify. Let's assume that element `e1` and `e2`
are neighbors and that they are adjacent across an element face that have the local indices `s1` and `s2` in elements
`e1` and `e2`, respectively. Element face `(e1, s1)` belongs to element `e1`, while element face `(e2, s2)` belongs to
element `e2`. This is important when applying boundary conditions on boundaries that are not domain boundaries. Let's say
element `e1` belongs to block `b1` and element `e2` belongs to block `b2` and we want to apply boundary conditions for the
heat equation solved on block `b1`. In this case, the boundary that we apply boundary conditions for the heat equation
must be composed of element faces belonging to `b1`. Going back to our two neighboring example elements, the face `(e1,s1)`
must be added to the boundary because it belongs to element `e1` which belongs to block `b1` which defines the temperature.
If the user tries to impose the boundary condition on element face `(e2,s1)` MOOSE will segfault. This distinction between
internal and external boundaries is important to understand how to define boundaries around radiation cavities.  

Boundaries enclosing a cavity are either external or internal boundaries. External boundaries
do not have a neighbor on the other side of the face, while internal boundaries have valid elements on
both sides of it. For external boundaries, the sideset must belong to one of the cavity blocks simply because there
are no elements on the other side of the sideset.

However, for internal sidesets the cavity boundary +must+ belong to the element right outside of the cavity.
If element face `(e1, s1)` is just outside of the cavity and `(e2, s2)` is immediately on the other side inside of the cavity,
then `(e1, s1)` should be used to construct the sideset. The reason is that internal sidesets will usually solve coupled
heat conduction problems on the block just outside the cavity and boundary conditions for the temperature defined on this
block must be imposed on the heat conduction/cavity interface.

The [SideSetsBetweenSubdomainsGenerator.md] mesh generator can be used for setting up
cavity boundaries. In this mesh generator, the sidesets will belong to the "primary" side. Returning to the example,
if [!param](/Mesh/SideSetsBetweenSubdomainsGenerator/primary_block) is `b1` and [!param](/Mesh/SideSetsBetweenSubdomainsGenerator/paired_block) is `b2`, then `(e1,s1)` will be the element faces used in the new sideset.
Specifically, the [!param](/Mesh/SideSetsBetweenSubdomainsGenerator/primary_block) are the blocks around the cavity, while [!param](/Mesh/SideSetsBetweenSubdomainsGenerator/paired_block) are the blocks inside the cavity.

## Example Input syntax

!listing modules/heat_conduction/test/tests/view_factors/view_factor_obstructed.i
block=UserObjects

!syntax parameters /UserObjects/RayTracingViewFactor

!syntax inputs /UserObjects/RayTracingViewFactor

!syntax children /UserObjects/RayTracingViewFactor
