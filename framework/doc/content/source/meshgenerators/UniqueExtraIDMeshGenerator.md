# UniqueExtraIDMeshGenerator

!syntax description /Mesh/UniqueExtraIDMeshGenerator


## Overview


`UniqueExtraIDMeshGenerator` object applies a new set of extra IDs determined by parsing existing extra IDs.
First, the exisiting extra IDs of each element are parsed to find a set of unique combinations of their ID values.
Then, a new ID value are determined to individual unique combiniation and assign it to the corresponding element.
The new ID is embedded in all the existing IDs, i.e. elements with a unique new ID value have a unique value for all existing IDs.

The `UniqueExtraIDMeshGenerator` requires the following parametes:

- [!param](/Mesh/UniqueExtraIDMeshGenerator/id_name): list of existing extra ID names.

- [!param](/Mesh/UniqueExtraIDMeshGenerator/new_id_name): name of the new extra ID name.

Optionally, users can control how the new ID values are determiend by using [!param](/Mesh/UniqueExtraIDMeshGenerator/new_id_rule), which is a vector of unsigned integers.
If [!param](/Mesh/UniqueExtraIDMeshGenerator/new_id_rule) is provided, new ID values are determined by mutiplying the provided integers to the correpsonding existing ID values and then summing the resulting values as: new_id_value = new_id_rule\[0\] * extra_id_value\[0\] + ... + new_id_rule\[n\] * extra_id_value\[n\].

## Example Syntax

!listing test/tests/meshgenerators/unique_extra_id_mesh_generator/unique_id.i block=Mesh

!syntax parameters /Mesh/UniqueExtraIDMeshGenerator

!syntax inputs /Mesh/UniqueExtraIDMeshGenerator

!syntax children /Mesh/UniqueExtraIDMeshGenerator
