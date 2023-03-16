# LayeredBase

Is a low-level base class containing the common logic for computing and storing spatial partial sums in a given cardinal direction.
It is intended to be derived from in any "Layered" calculation. The [LayeredIntegral.md] and derived classes compute and store
data structures for use in producing field values suitable for transfer where domains may not overlap or even represent similar areas,
but that contain some similar geometric relationship. For example, a "Layered" calculation may be useful for transfering heat produced
within a cylindrical object to the surrounding "outside", which may be a different shape or might just be represented by a 1D line.

The LayeredBase object derives from [Restartable.md] so it maintains stateful data across time steps.