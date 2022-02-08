# HeatStructureFromFile3D

This component allows users to load a mesh for the heat conduction model from a file.

The block, sideset and nodeset names are preserved, but prepended with a component name. For example,
if the component name is `hs` and the mesh has a block called `body`, users can refer to this block
using the `hs:body` name.  Similarly for side sets and node sets.

## Usage

The following parameters need to be specified:

- `position`: The location in 3D space where the origin of the mesh will be placed. This can be used to move the mesh from its original location specified in the file.
- `file`: the ExodusII file name with the mesh to load. This is the only supported format at the moment.

To define the material properties used by the heat conduction model, users must
create materials that define the following properties on all of their blocks:

- `density`
- `specific_heat`
- `thermal_conductivity`

!syntax parameters /Components/HeatStructureFromFile3D

!syntax inputs /Components/HeatStructureFromFile3D

!syntax children /Components/HeatStructureFromFile3D
