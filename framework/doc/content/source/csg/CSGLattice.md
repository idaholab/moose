# CSGLattice

This is an abstract class that can be used to define custom 2D lattice types for [constructive solid geometry (CSG)](syntax/CSG/index.md) representations.
The [source/csg/CSGBase.md] framework already contains definitions for Cartesian and regular hexagonal lattices, but developers of modules or MOOSE-based application can define additional custom types by deriving from this abstract class.
General information about implementing [!ac](CSG)-related methods can be found in [source/csg/CSGBase.md].

## Defining a New Lattice Type

Any custom lattice can be defined by inheriting from `CSGLattice`.
There are several methods that must be defined for any lattice type, each of which is elaborated on in the following sections:

- Public Methods:

  - `getAttributes`: returns a map of lattices attributes to their values
  - `isValidIndex`: checks if a given index is valid for the lattice dimensions
  - `isValidUniverseMap`: checks that a provided 2D set of universes is arranged in a valid form for the lattice type
  - `clone`: clones the instance as a unique pointer

- Protected Methods:

  - `setUniverses`: sets the 2D universe arrangement for the lattice
  - `compareAttributes`: checks if the attributes are identical to those of another lattice instance

### Defining a Valid Universe Layout

The rules of what makes a valid universe layout will vary depending on the lattice type.
These rules should be defined in the `isValidUniverseMap()`, which takes in a 2D vector of `CSGUniverse` objects and returns `true` or `false` depending on whether it is valid.
A recommendation for this method is to check that the number of rows and the length of each row is correct given the rules.

To set the value of the `_universe_map`, which is a 2D vector of `CSGUniverse` references that defines the layout, the `setUniverses()` method must be defined.
In this method, it is important to check the validity of the provided set of universes with the aforementioned `isValidUniverseMap()` method *and* set any necessary attributes that could change with the setting of the universe layout.
For example, in the [`CSGCartesianLattice` example](#example), the number of rows (`_nrow`) and columns (`_ncol`) need to be set in order to stay consistent with the lattice layout.

Each lattice should also have some sort of defined 2D indexing scheme that is used to define the layout and access items in the `_universe_map`.
The method `isValidIndex()` is where these rules are defined.
The `isValidIndex()` method is called by `setUniverseAtIndex()` and `getUniverseAtIndex()` to ensure that the provided index is valid.

### Lattice Attributes

The lattice attributes at a minimum are any pieces of data that are needed for a complete definition of the lattice for any downstream connected codes that cannot be derived from the `_universe_map` that defines the lattice layout.
For example, the `pitch` (flat-to-flat distance of elements) for the `CSGCartesianLattice` is needed for a complete definition, but cannot be determined from the universe layout directly.
These pieces of data can optionally be attributes that could be derived from the `_universe_map` but are just more convenient to have explicitly defined (e.g., for the [`CSGCartesianLattice` example](#example), the number of rows (`_nrow`) and columns (`_ncol`) can be derived from the universe layout, but they are included as attributes for convenience).
These attributes are what get returned in a map using the `getAttributes()` method, which gets called when producing the [!ac](CSG) [!ac](JSON) object in [source/csg/CSGBase.md].

!alert! note title=Attributes Data Type

The `getAttributes()` method returns a map of `std::any` data types (`std::unordered_map<std::string, std::any>`) to support flexibility in the type of data that might need to be defined.
However, this also means that to convert the information to the proper [!ac](JSON) data form, the data type needs to be explicitly determined.
Because of this, only certain data types are supported at this time: `int`, `unsigned int`, `Real`, `std::string`, and `bool`.

!alert-end!

These attributes are also what gets compared in the `compareAttributes()` method, which is used by the equality operators (`==` and `!=`) to check for lattice equality.
Because this data can be of any data type, this method must be explicitly defined for the lattice type, and use of `std::any_cast<type>` will likely be necessary to compare the values.

### Creating a Lattice Clone

In order to make sure that clones of `CSGBase` objects are created properly, each derived `CSGLattice` type must implement a `clone()` method, which returns a `std::unique_ptr<CSGLattice>` from the given lattice instance.
This can typically be done by calling `std::make_unique` on the constructor for the derived lattice type.

### Setting the Lattice Type

The type of lattice must be set for `_lattice_type` in the lattice constructor.
It is recommended that this be done based on the class name using `MooseUtils::prettyCppType<LatticeClassName>()` so that the lattice type automatically matches the class that created it.

## Example

Below shows how `CSGCartesianLattice` is implemented as an example.

!listing /framework/include/csg/CSGCartesianLattice.h

!listing /framework/src/csg/CSGCartesianLattice.C
