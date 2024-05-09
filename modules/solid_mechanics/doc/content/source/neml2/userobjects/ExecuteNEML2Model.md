# ExecuteNEML2Model

!syntax description /UserObjects/ExecuteNEML2Model

## Description

This object uses the specified NEML2 material model to perform mesh-wise (or subdomain-wise) batched
material update.

Each NEML2 model +input+ is gathered by a [MOOSEMaterialPropertyToNEML2](MOOSEMaterialPropertyToNEML2.md) or [MOOSEVariableToNEML2](MOOSEVariableToNEML2.md) user object given in [!param](/UserObjects/ExecuteNEML2Model/gather_uos).

Each model +output+ can be retireved by a [NEML2ToMOOSEMaterialProperty](NEML2ToMOOSEMaterialProperty.md) material object.

## Example Input Syntax

!listing modules/solid_mechanics/test/tests/neml2/neml2_to_moose_material.i block=UserObjects/model

!syntax parameters /UserObjects/ExecuteNEML2Model

!syntax inputs /UserObjects/ExecuteNEML2Model

!syntax children /UserObjects/ExecuteNEML2Model
