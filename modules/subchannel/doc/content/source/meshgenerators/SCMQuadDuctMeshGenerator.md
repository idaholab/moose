# SCMQuadDuctMeshGenerator

!syntax description /Mesh/SCMQuadDuctMeshGenerator

## Overview

!! Intentional comment to provide extra spacing

This mesh generator creates the mesh (with Quad4 elements) where the variables associated with the duct live.

The subdomain name used for the duct will match the name of this mesh generator.

## AuxVariables

Defining this mesh automatically creates the auxvariables detailed [here](SCMAuxVariables.md).

## Example Input File Syntax

!syntax parameters /Mesh/SCMQuadDuctMeshGenerator

!syntax inputs /Mesh/SCMQuadDuctMeshGenerator

!syntax children /Mesh/SCMQuadDuctMeshGenerator
