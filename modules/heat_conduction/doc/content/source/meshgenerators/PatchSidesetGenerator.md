# PatchSidesetGenerator

## Description

This mesh generator splits a given sideset (`sideset` parameter) into `n` pieces (`n_patches`).
The pieces are referred to as patches. Patches are used for net radiation transfer method via
view factors. The sideset is divided into `n` patches using partitioner that are available from
libmesh. The new sidesets are named `<old_side_name>_<id>`, where `<id>` is a number running from
`0` to `n - 1`.

## Example Input Syntax

!listing modules/heat_conduction/test/tests/generate_radiation_patch/generate_radiation_patch.i start=[patch] end=[] include-end=true

!syntax parameters /MeshGenerators/PatchSidesetGenerator

!syntax inputs /MeshGenerators/PatchSidesetGenerator

!syntax children /MeshGenerators/PatchSidesetGenerator
