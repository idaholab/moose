# PointInSignedFunctionCheckUO

`PointInSignedFunctionCheckUO` classifies a point against a surface defined
implicitly as the zero level set of a signed function. Unlike the mesh-based
[PointInPolyhedronCheckUO.md], it needs no surface mesh: the classification comes
directly from the sign of the function value at the query point. It implements the
point-in-surface check interface, so it can be combined with mesh-based checkers
inside a [PointInUnionCheckUO.md].

## Sign convention

By default a *negative* function value denotes the interior, matching the usual
signed-distance-function convention (and the union signed distance used elsewhere
in the shifted boundary method). If your level set is positive inside instead, set
`inside_is_negative = false` and the sign is flipped internally.

## Tolerance and the on-surface band

A point is classified as follows, after normalizing so that negative denotes the
interior:

- `phi < -tolerance` -> `INSIDE`
- `|phi| <= tolerance` -> `ON`
- `phi >  tolerance` -> `OUTSIDE`

Two consequences are worth noting:

- **`tolerance` affects containment.** Because `ON` is treated as contained
  (`spatialValue()` returns `1` for it), the contained region is the true interior
  *grown by a shell of half-width `tolerance`* around the zero level set, rather
  than the exact sign region. This matches how the mesh backend treats on-surface
  points.
- **`tolerance` is measured in function value space, not distance.** It equals a
  geometric distance only when the function is a normalized signed distance
  (`|grad phi| = 1`). For a general level set (for example `x^2 + y^2 - 1`, whose
  gradient magnitude varies) the geometric thickness of the on-surface band
  changes with position, so choose `tolerance` with the function's scaling in
  mind.

## Usage

Set `function` to the signed level-set function. The result is available through
the standard spatial user object interface and can be sampled by a
[SpatialUserObjectAux.md].

!syntax description /UserObjects/PointInSignedFunctionCheckUO

!syntax parameters /UserObjects/PointInSignedFunctionCheckUO

!syntax inputs /UserObjects/PointInSignedFunctionCheckUO

!syntax children /UserObjects/PointInSignedFunctionCheckUO
