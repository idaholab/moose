# MOOSEVariableToNEML2

!syntax description /UserObjects/MOOSEVariableToNEML2

## Description

This object collects a given [!param](/UserObjects/MOOSEVariableToNEML2/moose_variable) for use as an input [!param](/UserObjects/MOOSEVariableToNEML2/neml2_variable) to a NEML2 model executed by a [ExecuteNEML2Model](ExecuteNEML2Model.md) user object.

To use a MOOSE +material property+ as input to a NEML2 object see [MOOSEMaterialPropertyToNEML2](MOOSEMaterialPropertyToNEML2.md).

## Example Input Syntax

!listing modules/solid_mechanics/test/tests/neml2/neml2_to_moose_material.i block=UserObjects/gather_a

!syntax parameters /UserObjects/MOOSEVariableToNEML2

!syntax inputs /UserObjects/MOOSEVariableToNEML2

!syntax children /UserObjects/MOOSEVariableToNEML2
