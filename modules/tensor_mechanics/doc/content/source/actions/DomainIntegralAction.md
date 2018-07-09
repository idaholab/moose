# DomainIntegralAction

!syntax description /DomainIntegral/DomainIntegralAction

## Description

The `DomainIntegral` action is used to set up all of the objects used in computing all fracture domain integrals, including the $J$-integral, interaction integral, and T-stress. To use the fracture domain integrals, one must set up a model that incorporates a crack using one of two techniques:

Meshed crack: The crack can be explicity included by creating a mesh with a topology that conforms to the crack. The location of the crack tip is provided to the code by defining a nodeset that includes all nodes in the finite element mesh that are located along the crack front. For 2D analyses, this nodeset would only contain a single node at the crack tip.  For 3D analyses, the mesh connectivity is used to construct a set of line segments that connect these nodes, and this is used to order the crack nodes.

XFEM: Rather than defining the topology of the crack through the mesh, XFEM can be used to cut the mesh. In this case, a set of points, which does not need to conform to points in the mesh, must be provided by the user, and is used to define the location of the crack for coumputing the fracture integrals. Fracture integrals are computed at the locations of these points, in the order provided by the user.

!syntax parameters /DomainIntegral/DomainIntegralAction
