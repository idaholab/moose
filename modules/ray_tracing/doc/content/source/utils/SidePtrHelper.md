# SidePtrHelper

## Description

`SidePtrHelper` is a helper class for generating `Elem` objects that represent the side element of another `Elem`.

The standard method of building an element side in [libMesh] is `Elem::side_ptr()`. This method constructs a *new* `Elem` object every time it is called, which can be expensive if done repeatedly.

`SidePtrHelper` cheapens this process by retaining its own `Elem` objects, which have their points changed on request of an element side.