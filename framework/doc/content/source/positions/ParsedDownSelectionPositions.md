# ParsedDownSelectionPositions

!syntax description /Positions/ParsedDownSelectionPositions

The [!param](/Positions/ParsedDownSelectionPositions/expression) parameter should evaluate above 0 for an
element centroid to be selected as a `Position`. The `x`, `y`, `z` and `t` symbols for position and time
may be used in the `expression` without declaring them as functors.

Functors may be added directly in the `expression` using [!param](/Positions/ParsedDownSelectionPositions/functor_names), or
may be  aliased using the [!param](/Positions/ParsedDownSelectionPositions/functor_symbols) parameters.
The alias can be useful for names containing the `:` character which may not be used in the parsed `expression`.

!alert note
The `ParsedDownSelectionPositions` may be block-restricted in addition to the parsed expression evaluation criterion.

!syntax parameters /Positions/ParsedDownSelectionPositions

!syntax inputs /Positions/ParsedDownSelectionPositions

!syntax children /Positions/ParsedDownSelectionPositions
