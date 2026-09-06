# PointInUnionCheckUO

`PointInUnionCheckUO` classifies a point against the *union* of several
geometries. Each geometry is supplied by another user object listed in the
`providers` parameter, and every provider must implement the point-in-surface
check interface. The providers may be given in mixed representations: a meshed
closed surface ([PointInPolyhedronCheckUO.md]), a signed level-set function
([PointInSignedFunctionCheckUO.md]), or another `PointInUnionCheckUO`.

## Union semantics

The classification is tri-state and follows the precedence
**INSIDE > ON > OUTSIDE**:

- the point is `INSIDE` if it is inside any one geometry;
- otherwise `ON` if it is on any geometry;
- otherwise (outside every geometry) `OUTSIDE`.

Equivalently, the boolean union is "inside any, outside every": a point is
contained (`spatialValue()` returns `1`) when it is inside or on at least one
geometry, and `0` only when it is outside all of them.

### Approximate union boundary (not CSG)

The precedence rule reproduces the true set union of the *contained regions*, and
correctly resolves overlaps: a point that lies on the surface of one geometry but
inside another is reported `INSIDE`, because it is interior to the union. It does
not, however, compute the exact boundary of the union. Where two surfaces cross,
a point may be reported `ON` even though it is not on the true union boundary.
Computing the exact union boundary would require constructive solid geometry
(CSG), which is outside the scope of this object. For in-out testing this
approximation is sufficient.

## Nesting

Because `PointInUnionCheckUO` itself implements the point-in-surface check
interface, a union may be listed as a provider of another union. This lets a
composite geometry be built up in stages.

## Usage

List the provider user objects in `providers`; the list must be non-empty (an
empty list is rejected, since it would classify every point as outside). The
result is available through the standard spatial user object interface, so a
[SpatialUserObjectAux.md] evaluates it at nodes or element centroids and stores
`1`/`0` in an auxiliary variable.

!syntax description /UserObjects/PointInUnionCheckUO

!syntax parameters /UserObjects/PointInUnionCheckUO

!syntax inputs /UserObjects/PointInUnionCheckUO

!syntax children /UserObjects/PointInUnionCheckUO
