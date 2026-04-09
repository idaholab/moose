# STLSubdomainGenerator

!syntax description /Mesh/STLSubdomainGenerator

## Overview

`STLSubdomainGenerator` assigns a subdomain ID based on whether an element centroid lies inside a
closed STL manifold. The STL surface must be watertight and manifold. Element centroids that lie on
the STL surface within the configured tolerance are treated as inside.

This generator reads an STL file directly for containment tagging only. It does not make STL a
general-purpose input format for [FileMeshGenerator](FileMeshGenerator.md).

## Example

The example below creates a structured 3D mesh, translates a cube STL into the mesh domain, and
assigns subdomain `1` to the elements whose centroids lie inside the closed surface.

!listing test/tests/meshgenerators/stl_subdomain_generator/basic_inside.i block=Mesh

The same STL geometry can be rescaled with
[!param](/Mesh/STLSubdomainGenerator/scale), repositioned with
[!param](/Mesh/STLSubdomainGenerator/rotation), and shifted with
[!param](/Mesh/STLSubdomainGenerator/translation). The transform order is scale,
then rotation, then translation. Users may also limit reassignment to selected
existing blocks with
[!param](/Mesh/STLSubdomainGenerator/restricted_subdomains).

!syntax parameters /Mesh/STLSubdomainGenerator

!syntax inputs /Mesh/STLSubdomainGenerator

!syntax children /Mesh/STLSubdomainGenerator
