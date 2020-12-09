# ElemExtrema

## Description

`ElemExtrema` is a helper struct for identifying if a [Ray.md] has intersected the "extrema" of an element during tracing. In 2D, an element extrema is defined as a vertex. In 3D, an element extrema is defined as a vertex or an edge.

The intersected element extrema is avaiable in [RayKernels/index.md] and [RayBCs/index.md] during a trace as the `_current_intersected_extrema` member variable.

The methods of interest for use during tracing on `ElemExtrema` are:

- `atExtrema()` - Whether or not the interesction is at an extrema.
- `atVertex()` - Whether or not the intersection is at a vertex.
- `atEdge()` - Whether or not the intersection is at an edge.
- `vertex()` - The local vertex index that was intersected.
- `vertexPoint()` - The point of the vertex that was intersected.
- `edgeVertices()` - The vertices that define the edge that was intersected.
- `buildEdge()` - Builds an `Elem` that is the edge that was intersected.