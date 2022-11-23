# ParsedNodeTransformGenerator

!syntax description /Mesh/ParsedNodeTransformGenerator

## Overview

The `ParsedNodeTransformGenerator` generator allows the user to specify parsed expressions to compute the new positions of all mesh nodes as the function of their old positions.
The old node positions components are made available as function symbold `x`, `y`, and `z`. The new node position components are determined by the [!param](/Mesh/ParsedNodeTransformGenerator/x_function),
[!param](/Mesh/ParsedNodeTransformGenerator/y_function), and [!param](/Mesh/ParsedNodeTransformGenerator/z_function) parmeters. The parsed expression syntax available is the same as used
in [`ParsedFunction`](MooseParsedFunction.md) and [`ParsedMaterial`](ParsedMaterial.md).


!syntax parameters /Mesh/ParsedNodeTransformGenerator

!syntax inputs /Mesh/ParsedNodeTransformGenerator

!syntax children /Mesh/ParsedNodeTransformGenerator
