# CoarseMeshExtraElementIDGenerator

!syntax description /Mesh/CoarseMeshExtraElementIDGenerator

## Overview

This mesh generator adds an extra element integer to an input mesh based on a coarse mesh.
The extra element integer of an element on the input mesh is assigned with the element ID of the coarse element containing the centroid of the fine element by default.
Users can use the [!param](/Mesh/CoarseMeshExtraElementIDGenerator/coarse_mesh_extra_element_id) parameter to use an extra element ID on the coarse mesh for the assignment instead of the default coarse element ID.
A reserved name *subdomain_id* can be used for [!param](/Mesh/CoarseMeshExtraElementIDGenerator/coarse_mesh_extra_element_id) for using the subdomain ID on the coarse mesh.
Users can optionally check if the fine mesh is embedded in the coarse mesh using the [!param](/Mesh/CoarseMeshExtraElementIDGenerator/enforce_mesh_embedding) parameter, i.e. every element on the fine mesh must be contained
within one and only one coarse element or a region by [!param](/Mesh/CoarseMeshExtraElementIDGenerator/coarse_mesh_extra_element_id) of the coarse mesh.
The extra element ID assignment can be restricted to subdomains in the fine mesh by specifying [!param](/Mesh/CoarseMeshExtraElementIDGenerator/subdomains). With this parameter, this mesh generator can be called multiple times with different coarse meshes. After the first assignment, subsequent IDs will be offset by the maximum ID found from previous calls.

!syntax parameters /Mesh/CoarseMeshExtraElementIDGenerator

!syntax inputs /Mesh/CoarseMeshExtraElementIDGenerator

!syntax children /Mesh/CoarseMeshExtraElementIDGenerator
