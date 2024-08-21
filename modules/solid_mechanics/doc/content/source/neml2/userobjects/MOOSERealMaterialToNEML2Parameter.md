# MOOSERealMaterialToNEML2Parameter

!syntax description /UserObjects/MOOSERealMaterialToNEML2Parameter

## Description

This object collects a given [!param](/UserObjects/MOOSERealMaterialToNEML2Parameter/moose_material_property) for use as a parameter [!param](/UserObjects/MOOSERealMaterialToNEML2Parameter/neml2_parameter) in a NEML2 model executed by a [ExecuteNEML2Model](ExecuteNEML2Model.md) user object.

!alert note title=Note
Only Real material type is supported in this object.

To use a MOOSE +variable+ as parameter in a NEML2 object see [MOOSEVariableToNEML2Parameter](MOOSEVariableToNEML2Parameter.md).

## Example Input Syntax

!listing modules/solid_mechanics/test/tests/neml2/moose_to_neml2_parameters.i block=UserObjects/gather_nu

!syntax parameters /UserObjects/MOOSERealMaterialToNEML2Parameter

!syntax inputs /UserObjects/MOOSERealMaterialToNEML2Parameter

!syntax children /UserObjects/MOOSERealMaterialToNEML2Parameter
