# SCMTriDuctMeshGenerator

!syntax description /Mesh/SCMTriDuctMeshGenerator

## Overview

!! Intentional comment to provide extra spacing

This mesh generator creates the mesh (with Quad4 elements) where the variables associated with the duct live.

The subdomain name used for the duct will match the name of this mesh generator.

## Example Input File Syntax

!listing /examples/duct/test.i block=TriSubChannelMesh language=moose

!syntax parameters /Mesh/SCMTriDuctMeshGenerator

!syntax inputs /Mesh/SCMTriDuctMeshGenerator

!syntax children /Mesh/SCMTriDuctMeshGenerator
