# ElementIntegerAux

!syntax description /AuxKernels/ElementIntegerAux

Additional integer fields may be used to group element separately from the subdomain/block.
The `ElementIntegerAux` allows to visualize this additional information stored on the
mesh.

## Example syntax

In this example, the `ElementIntegerAux` is used to retrieve the `material_id` additional
field and store it in the `id` auxiliary variable.

!listing test/tests/auxkernels/mesh_integer/dg_mesh_integer.i block=AuxKernels

!syntax parameters /AuxKernels/ElementIntegerAux

!syntax inputs /AuxKernels/ElementIntegerAux

!syntax children /AuxKernels/ElementIntegerAux
