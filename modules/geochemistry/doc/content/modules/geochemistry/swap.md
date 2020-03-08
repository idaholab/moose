# Basis swaps

Notation and definitions are described in [geochemistry_nomenclature.md].

It is useful to swap constituents in and out of the [basis](basis.md).  For instance:

- In the problem definition, it may be more convenient to include a mineral or a gas species as part of the basis, instead of the default primary species.
- After the iterative procedure has "converged", the molality of a mineral may be negative, in which case it is removed from the basis and replaced by a secondary species (and the iterative procedure re-done).
- After the iterative procedure has "converged", a mineral may be supersaturated.  It is then added to the basis, and either a primary species or another mineral is removed (and the iterative procedure re-done).
- A species in the basis can occur at very small concentration, leading to numerical instability because making small corrections to its molality leads to large deviations in the molalities of the secondary species.  In this case it can be swapped for a more abundant secondary species.

Chapter 5 of [!cite](bethke_2007) describes this swapping procedure in detail.  This includes a test for a valid basis swap, a procedure for re-writing the chemical reactions in terms of the new basis, the computation of the new equilibrium constants, and writing the system's bulk composition ($M$) in the new basis.

!bibtex bibliography
