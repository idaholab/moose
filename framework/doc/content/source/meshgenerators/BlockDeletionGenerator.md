# BlockDeletionGenerator

!syntax description /Mesh/BlockDeletionGenerator

## Overview

The `BlockDeletionGenerator` is used to remove elements from a mesh matching a
user provided subdomain ID. While most mesh generation operations should be
based on geometric operations, it is sometimes easier to remove discretized
elements, by blocks, for certain problems.

For example, if we have a mesh that models both a pipe and its interior,
but we only want to model the fluid flow, we may delete the subdomain associated
with the pipe.

!alert note
Once a block is deleted from the mesh, it should not be referred to in the input.
Variables and materials can no longer be block restricted to a deleted block, for example.

!alert note
Lower-dimensional elements created from the deleted elements will also be removed.

## Example input syntax

In this example input file, we remove blocks 1 and 3 with a single `BlockDeletionGenerator`.
This leaves only block 2 in the simulation.

!listing test/tests/meshgenerators/block_deletion_generator/block_deletion_test14.i block=Mesh

!syntax parameters /Mesh/BlockDeletionGenerator

!syntax inputs /Mesh/BlockDeletionGenerator

!syntax children /Mesh/BlockDeletionGenerator
