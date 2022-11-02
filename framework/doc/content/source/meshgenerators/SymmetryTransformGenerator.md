# SymmetryTransformGenerator

!syntax description /Mesh/SymmetryTransformGenerator

!alert note
Only the plane or mirror symmetry is currently implemented in the `SymmetryTransformGenerator`. Point or rotational symmetries are not implemented.

## Overview

The `SymmetryTransformGenerator` makes a mirror reflection of a mesh across an arbitrary plane (or line in 2D) supplied by the user.
All input is expected to be three dimensional even if the mesh in question is two dimensional; in such a case,
let the z component be 0 (for example, `mirror_normal_vector = '1 1 0'`).

The user sets the plane that will be reflected over by giving two vectors: a vector that gives the position of
any given point on the line/plane from the origin; and, a vector that is normal (aka perpendicular/ orthogonal)
to said line/plane. The normal vector establishes the slope of the plane of reflection. 

!syntax parameters /Mesh/SymmetryTransformGenerator

!syntax inputs /Mesh/SymmetryTransformGenerator

!syntax children /Mesh/SymmetryTransformGenerator
