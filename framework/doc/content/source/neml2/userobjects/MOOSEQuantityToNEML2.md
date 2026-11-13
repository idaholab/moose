# MOOSEQuantityToNEML2

!if! function=hasCapability('neml2')

!alert note
Users are +NOT+ expected to directly use this object in an input file. Instead, it is always recommended to use the [NEML2 action](syntax/NEML2/index.md).

## Description

This object collects a MOOSE quantity given by [!param](/UserObjects/MOOSERealToNEML2/from_moose) for use as a NEML2 input variable or model parameter [!param](/UserObjects/MOOSERealToNEML2/to_neml2). The source of data is specified by [!param](/UserObjects/MOOSERealToNEML2/quantity_type). This object has an "old" counterpart to retrieve the corresponding MOOSE data from the previous time step. The naming convention is

Data is gathered in element interiors when [!param](/UserObjects/MOOSERealToNEML2/volume) is `true`,
on the sides listed in `external_boundaries`, and on both sides of the internal sidesets listed in
`interface_boundaries`. Material properties are read from the volume, face, or neighbor material data
that matches each location, or from interface material data when
[!param](/UserObjects/MOOSERealToNEML2/from_interface_material) is `true`. See
[evaluation domains](syntax/NEML2/index.md#domains) for details.

!syntax parameters /UserObjects/MOOSERealToNEML2

!if-end!

!else

!include neml2/neml2_warning.md
