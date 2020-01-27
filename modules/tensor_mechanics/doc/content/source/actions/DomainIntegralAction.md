# DomainIntegralAction

!syntax description /DomainIntegral/DomainIntegralAction

## Description

The `DomainIntegral` action is used to set up all of the objects used in computing all fracture domain integrals, including the $J$-integral, interaction integral, and T-stress. To use the fracture domain integrals, one must set up a model that incorporates a crack using one of two techniques:

Meshed crack: The crack can be explicity included by creating a mesh with a topology that conforms to the crack. The location of the crack tip is provided to the code by defining a nodeset that includes all nodes in the finite element mesh that are located along the crack front. For 2D analyses, this nodeset would only contain a single node at the crack tip.  For 3D analyses, the mesh connectivity is used to construct a set of line segments that connect these nodes, and this is used to order the crack nodes.

XFEM: Rather than defining the topology of the crack through the mesh, XFEM can be used to cut the mesh. In this case, a set of points, which does not need to conform to points in the mesh, must be provided by the user, and is used to define the location of the crack for coumputing the fracture integrals. Fracture integrals are computed at the locations of these points, in the order provided by the user.

## Theory

Details on the theory behind the computation of the various fracture integrals are provided [here](FractureIntegrals.md).

## Objects created by this Action

!table id=DomainIntegralActionIntegrals caption=Classes used to perform various fracture integrals specified through the `integrals` parameter
| Class                                            | Type                | Functionality |
|--------------------------------------------------|---------------------|---------------|
| [JIntegral](JIntegral.md)                        | VectorPostprocessor | Computes $J$-Integral at all points on crack front for a given integration ring |
| [InteractionIntegral](InteractionIntegral.md)    | VectorPostprocessor | Computes $K_I$, $K_{II}$, $K_{III}$, or $T$ stress using interaction integral at all points on crack front for a given integration ring |
| [MixedModeEquivalentK](MixedModeEquivalentK.md)  | VectorPostprocessor | Computes an exquivalent stress intensity factor from $K_I$, $K_{II}$, and $K_{III}$ |
| [CrackFrontDefinition](CrackFrontDefinition.md)  | UserObject | Defines crack front and provides geometry-related functions used by [JIntegral](JIntegral.md) and [InteractionIntegral](InteractionIntegral.md) |
| [VectorPostprocessorComponent](VectorPostprocessorComponent.md) | Postprocessor | Reports individual quantities computed by VectorPostprocessors at a specific point on the crack front |
| [CrackFrontData](CrackFrontData.md) | Postprocessor | Reports values of requested variables at crack front points |
| [VectorOfPostprocessors](VectorOfPostprocessors.md) | VectorPostprocessor | Assembles CrackFrontData values into a vector for output |
| [ThermalFractureIntegral](ThermalFractureIntegral.md) | Material | Computes derivative of all eigenstrains with respect to temperature for use in fracture integrals |
| [StrainEnergyDensity](StrainEnergyDensity.md) | Material | Computes strain energy density for use in fracture integrals |
| [DomainIntegralQFunction](DomainIntegralQFunction.md) | AuxKernel | Optionally populates AuxVariables with values of the geometry-based q function when `output_q=true` |
| [DomainIntegralTopologicalQFunction](DomainIntegralTopologicalQFunction.md) | AuxKernel | Optionally populates AuxVariables with values of the topology-based q function when `output_q=true` |

!syntax parameters /DomainIntegral/DomainIntegralAction
