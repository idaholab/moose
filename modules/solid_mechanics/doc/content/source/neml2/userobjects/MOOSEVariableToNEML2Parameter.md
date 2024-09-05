# MOOSEVariableToNEML2Parameter

!syntax description /UserObjects/MOOSEVariableToNEML2Parameter

## Description

This family of objects collects a given [!param](/UserObjects/MOOSEVariableToNEML2Parameter/moose_variable) for use as parameter [!param](/UserObjects/MOOSEVariableToNEML2Parameter/neml2_parameter) in a NEML2 model executed by a [ExecuteNEML2Model](ExecuteNEML2Model.md) user object.

To use a MOOSE +material+ as parameter in a NEML2 object see [MOOSERealMaterialToNEML2Parameter](MOOSERealMaterialToNEML2Parameter.md).

## Example Input Syntax

!listing modules/solid_mechanics/test/tests/neml2/moose_to_neml2_parameters.i block=UserObjects/gather_E

!syntax parameters /UserObjects/MOOSEVariableToNEML2Parameter

!syntax inputs /UserObjects/MOOSEVariableToNEML2Parameter

!syntax children /UserObjects/MOOSEVariableToNEML2Parameter
