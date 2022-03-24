# ElementsAlongLine

!syntax description /VectorPostprocessors/ElementsAlongLine

## Description

The `ElementsAlongLine` class is a VectorPostprocessor that outputs the element ID of
every element intersected by a line. The IDs are provided in a vector named `elem_ids`.

The user defines the line using a start and end point. The line terminates at those points,
so elements on the line beyond those points are not output.

The IDs output from this class use the MOOSE internal numbering scheme, which starts
with 0, so 1 should be added to them to translate them to the equivalent numbering in
formats such as Exodus that start with 1.

!syntax parameters /VectorPostprocessors/ElementsAlongLine

!syntax inputs /VectorPostprocessors/ElementsAlongLine

!syntax children /VectorPostprocessors/ElementsAlongLine
