# ElementGenerator

!syntax description /Mesh/ElementGenerator

## Overview

This MeshGenerator object allows the user to create a single element (it is possible to create a simple mesh by adding elements one by one).

The user has to provide the positions of the nodes for their element, as well as the type of element they want (QUAD4, TRI3,...) and the element node connectivity.

!alert note
The reader is referred to the libmesh documentation and doxygen for the convention on node connectivity to create a
well formed, positive-volume element.

## Further ElementGenerator Documentation

!syntax parameters /Mesh/ElementGenerator

!syntax inputs /Mesh/ElementGenerator

!syntax children /Mesh/ElementGenerator
