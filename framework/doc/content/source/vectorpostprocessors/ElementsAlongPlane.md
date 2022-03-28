# ElementsAlongPlane

!syntax description /VectorPostprocessors/ElementsAlongPlane

## Description

The `ElementsAlongPlane` class is a VectorPostprocessor that outputs the element ID of
every element intersected by a plane. The IDs are provided in a vector named `elem_ids`.

The user defines the plane using a combination of a point on the plane and a normal to
the plane, and the plane extends infinitely.

The IDs output from this class use the MOOSE internal numbering scheme, which starts
with 0, so 1 should be added to them to translate them to the equivalent numbering in
formats such as Exodus that start with 1.

!syntax parameters /VectorPostprocessors/ElementsAlongPlane

!syntax inputs /VectorPostprocessors/ElementsAlongPlane

!syntax children /VectorPostprocessors/ElementsAlongPlane
