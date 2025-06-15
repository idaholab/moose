# ParsedSelectionPositions

!syntax description /Positions/ParsedSelectionPositions

## Overview

The `ParsedSelectionPositions` may be block-restricted to limit the gathering of elements
to certain subdomains.

The [!param](/Positions/ParsedSelectionPositions/expression) parameter should evaluate above 0 for an
element centroid to be selected as a `Position`. The `x`, `y`, `z` and `t` symbols for position and time
may be used in the `expression` without declaring them as functors.

Functors may be added directly in the `expression` using [!param](/Positions/ParsedSelectionPositions/functor_names), or
may be  aliased using the [!param](/Positions/ParsedSelectionPositions/functor_symbols) parameters.
The alias can be useful for names containing the `:` character which may not be used in the parsed `expression`.

!syntax parameters /Positions/ParsedSelectionPositions

!syntax inputs /Positions/ParsedSelectionPositions

!syntax children /Positions/ParsedSelectionPositions
