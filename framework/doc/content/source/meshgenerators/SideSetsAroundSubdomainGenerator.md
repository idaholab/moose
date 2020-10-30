# SideSetsAroundSubdomainGenerator

!syntax description /Mesh/SideSetsAroundSubdomainGenerator

## Overview

By default, `SideSetsAroundSubdomainGenerator` creates a sideset (with ID defined
by the user) around an associated block ID. Multiple sidesets over multiple blocks
can be defined at once.

Optional parameters allow more specific behavior:

- A normal vector can be defined (using `normal`) such that only faces whose outward facing normal is equivalent (or within a specified tolerance `normal_tol`) is added to the sideset.

- `replace` is a boolean parameter that sets whether any old sidesets should be replaced or preserved (default is preservation).

- `fixed_normal` is a boolean parameter inherited from `SideSetsGeneratorBase` that sets whether a sideset defined on a qualifying face is to be "painted" onto adjacent faces (default = False). This allows sidesets for slightly curved boundaries to be more easily defined. Fixing the normal (setting `fixed_normal = True`) disables this behavior.

!syntax parameters /Mesh/SideSetsAroundSubdomainGenerator

!syntax inputs /Mesh/SideSetsAroundSubdomainGenerator

!syntax children /Mesh/SideSetsAroundSubdomainGenerator
