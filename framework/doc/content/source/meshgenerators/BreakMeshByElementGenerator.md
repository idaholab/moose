# BreakMeshByElementGenerator

!syntax description /Mesh/BreakMeshByElementGenerator

## Overview

The `BreakMeshByElementGenerator` is used to break all element-element interfaces in specified subdomains. All element-element interfaces are grouped into a boundary of user's choice.

## Example input syntax

In this example input file, we break all element-element interfaces in subdomains 1 and 2.

!listing test/tests/meshgenerators/explode_mesh_generator/2D.i block=Mesh

!syntax parameters /Mesh/BreakMeshByElementGenerator

!syntax inputs /Mesh/BreakMeshByElementGenerator

!syntax children /Mesh/BreakMeshByElementGenerator
