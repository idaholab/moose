# DisplayGhostingAction

The `DisplayGhostingAction` is for outputting the ghosting of your simulation. In MOOSE, we attempt
to ghost only the minimum amount of data for running the simulation. In order to know what ghosting
we need each object can add one or more `RelationshipManager` classes. This object sets up
several `AuxVariables` and `AuxKernels` for outputting both the "Geometric" and "Algebraic" ghosting.

To output ghosting add the following parameter(s) to your Mesh block:
```
[Mesh]
  ...
  output_ghosting = true
  include_local_in_ghosting = true
[]
```

The `include_local_in_ghosting` parameter will also include all of a processors local elements
in the ghosting field to give you a complete view of ghosting.

Example:

!listing test/tests/auxkernels/ghosting_aux/ghosting_aux.i block=Mesh

!bibtex bibliography
