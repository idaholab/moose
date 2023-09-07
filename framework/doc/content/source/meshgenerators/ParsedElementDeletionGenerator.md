# ParsedElementDeletionGenerator

!syntax description /Mesh/ParsedElementDeletionGenerator

## Overview

The `ParsedElementDeletionGenerator` allows the user to specify a parsed criterion for the deletion of
an element.

The components of the vertex-average of each element are made available as function symbols `x`, `y`, and `z`.
The volume of each element is made available as function symbol `volume`.

The parsed expression syntax available is the same as used
in [`ParsedFunction`](MooseParsedFunction.md) and [`ParsedMaterial`](ParsedMaterial.md).

!alert warning
The criterion for deletion is that the expression evaluates to something that is strictly positive. For example,
`volume < 1e-10` will delete all elements below that volume.

!syntax parameters /Mesh/ParsedElementDeletionGenerator

!syntax inputs /Mesh/ParsedElementDeletionGenerator

!syntax children /Mesh/ParsedElementDeletionGenerator
