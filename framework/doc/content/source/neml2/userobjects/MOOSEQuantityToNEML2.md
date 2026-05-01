# MOOSEMaterialPropertyToNEML2

!if! function=hasCapability('neml2')

!alert note
Users are +NOT+ expected to directly use this object in an input file. Instead, it is always recommended to use the [NEML2 action](syntax/NEML2/index.md).

## Description

This object collects a MOOSE quantity given by [!param](/UserObjects/MOOSEQuantityToNEML2/from_moose) for use as a NEML2 input variable or model parameter [!param](/UserObjects/MOOSEQuantityToNEML2/to_neml2). The source of data is specified by [!param](/UserObjects/MOOSEQuantityToNEML2/quantity_type). This object has an "old" counterpart to retrieve the corresponding MOOSE data from the previous time step. The naming convention is

!syntax parameters /UserObjects/MOOSEQuantityToNEML2

!if-end!

!else

!include neml2/neml2_warning.md
