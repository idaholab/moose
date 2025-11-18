# SBMBndEdge2

`SBMBndEdge2` specializes [`SBMBndElementBase`](SBMBndElementBase.md) for 2-node edge
elements. This wrapper assumes that the edge lies parallel to the $x$-$y$ plane (i.e., the two
nodes share the same $z$ coordinate), ensuring that computed normals and intersection tests
remain strictly two-dimensional. The constructor verifies that the wrapped element is of
type `EDGE2` and satisfies this planarity condition.

!syntax description /SBMBndElement/SBMBndEdge2

!syntax parameters /SBMBndElement/SBMBndEdge2

!syntax inputs /SBMBndElement/SBMBndEdge2

!syntax children /SBMBndElement/SBMBndEdge2
