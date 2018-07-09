# NumElems

The NumElems Postprocessor provides information about the number of elements in the simulation. This postprocessor
is capable of providing either the active number of elements in the simulation (i.e. only the elements that are
being used for calculations), or the total number of elements (i.e. includes parent elements of refined elements,
which are maintained for the purpose of coarsening).

!alert note
This postprocessor returns the aggregate number of elements when using DistributedMesh.

## Syntax and Description

!syntax description /Postprocessors/NumElems

!syntax parameters /Postprocessors/NumElems

!syntax inputs /Postprocessors/NumElems

!syntax children /Postprocessors/NumElems

!bibtex bibliography
