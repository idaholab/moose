# ComputeNodePosition

!syntax description /Mesh/ComputeNodePosition

## Overview

The `ComputeNodePosition` generator allows the user to specify parsed expressions to compute the new positions of all mesh nodes as the function of their old positions.
The old node positions components are made available as function symbold `x`, `y`, and `z`. The new node position components are determined by the [!param](/Mesh/ComputeNodePosition/x_function),
[!param](/Mesh/ComputeNodePosition/y_function), and [!param](/Mesh/ComputeNodePosition/z_function) parmeters. The parsed expression syntax available is the same as used
in [`ParsedFunction`](MooseParsedFunction.md) and [`ParsedMaterial`](ParsedMaterial.md).


!syntax parameters /Mesh/ComputeNodePosition

!syntax inputs /Mesh/ComputeNodePosition

!syntax children /Mesh/ComputeNodePosition
