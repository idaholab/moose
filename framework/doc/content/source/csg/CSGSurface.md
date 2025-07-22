# CSGSurface

This is an abstract class for any surface definition for [constructive solid geometry (CSG)](syntax/CSG/index.md) representations.
The [source/csg/CSGBase.md] framework already contains definitions for some basic surfaces (planes, spheres, and cylinders), but developers of modules or MOOSE-based application can define additional custom surfaces using this abstract class.
General information about implementing [!ac](CSG)-related methods can be found in [source/csg/CSGBase.md].

## Defining a New Surface Type

Any arbitrary or custom surface can be defined by inheriting from `CSGSurface`.
At a minimum, the surface name, surface type, and boundary type, must be set, and two virtual methods that need to be defined.
Additional information required by the constructor will depend on the specific surface to be defined.

### Setting Coefficients

Each surface should be defined by some equation, which therefore requires the definition of coefficients or other parameters.
These coefficient values should be set and be returned through the `getCoefficients` method which will return a map of the coefficient strings to their values.
These coefficient values are returned in the [!ac](JSON) file that is produced.
For example, a plane is defined by the equation $ax + by + cz = d$ and so this method would return a map of the values for `a`, `b`, `c`, and `d`.

### Half-space Determination

In [!ac](CSG) representation, knowing which half-space of the surface is positive or negative is necessary for correct construction of a [cell](source/csg/CSGBase.md#cells) [region](source/csg/CSGBase.md#regions).
To help determine the sign of these half-spaces, each surface type should have a `getHalfspaceFromPoint` method implemented that can be used to determine if a point lies within the positive (`CSGSurface::Direction::POSITIVE`) or negative (`CSGSurface::Direction::NEGATIVE`) half-space for the surface.

### Setting the Surface Type

The type of surface must be set for `_surface_type` in the surface constructor.
It is recommended that this be done based on the class name using `MooseUtils::prettyCppType<SurfaceClassName>()` so that the surface type automatically matches the class that created it.

## Example

Below shows how `CSGSphere` is implemented as an example.

!listing /framework/include/csg/CSGSphere.h

!listing /framework/src/csg/CSGSphere.C
