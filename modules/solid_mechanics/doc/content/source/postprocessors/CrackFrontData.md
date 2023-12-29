# Crack Front Data

!syntax description /Postprocessors/CrackFrontData

## Description

This object is a utility function used to output values of nodal variables at nodes along a crack front used in a fracture domain integral calculation. This is useful for tying information about solution variables to the computed fracture integrals at specific points. The definition of the crack points must be provided using a [CrackFrontDefinition](/CrackFrontDefinition.md) object, which is typically set up automatically using a [DomainIntegralAction](/DomainIntegralAction.md).

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/j_integral/j_integral_3d.i block=Postprocessors/disp_x_centercrack

!syntax parameters /Postprocessors/CrackFrontData

!syntax inputs /Postprocessors/CrackFrontData

!syntax children /Postprocessors/CrackFrontData
