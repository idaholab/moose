# AllSideSetsByNormalsGenerator

!syntax description /Mesh/AllSideSetsByNormalsGenerator

## Overview

This mesh generator is useful for adding all possible sidesets to a mesh based on unique normal
directions.  It works well for more regular shapes that don't have curves or many disjoint surfaces
with similar normals.  If desired however, it is capable of following a slowly changing normal around
a surface so that a curve can be assigned a single sideset. This is accomplished by setting
*fixed_normal* to false. In this case a cylinder mesh can be given exactly three normals, one for the
two and bottom respectively, and a third normal for the curved surface of the cylinder.

!alert note
The sideset number assignment is not predictable. This utility assigns sideset numbering based on the
unique normals seen while iterating over the mesh. Further enhancements could be made to more
carefully control this assignment.

!syntax parameters /Mesh/AllSideSetsByNormalsGenerator

!syntax inputs /Mesh/AllSideSetsByNormalsGenerator

!syntax children /Mesh/AllSideSetsByNormalsGenerator
