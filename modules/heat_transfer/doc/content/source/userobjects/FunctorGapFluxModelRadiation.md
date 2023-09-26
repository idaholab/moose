# FunctorGapFluxModelRadiation

!syntax description /UserObjects/FunctorGapFluxModelRadiation

## Description

`FunctorGapFluxModelRadiation` implements the same equations as
[GapFluxModelRadiation.md], however, it uses the functor system to evaluate
needed quantities on-the-fly. The functor system is leveraged heavily by MOOSE's
finite volume discretizations. To use pre-initialized data, which is the
tradition for finite element discretizations, the [GapFluxModelRadiation.md]
object may be the more appropriate object to use.

## Example Input File Syntax

!listing modules/heat_conduction/test/tests/gap_heat_transfer_mortar/fv_modular_gap_heat_transfer_mortar_radiation_conduction.i block=UserObjects/radiation

`FunctorGapFluxModelRadiation` must be used in conjunction with the modular gap conductance
constraint as shown below:

!listing modules/heat_conduction/test/tests/gap_heat_transfer_mortar/fv_modular_gap_heat_transfer_mortar_radiation_conduction.i block=Constraints/ced

!syntax parameters /UserObjects/FunctorGapFluxModelRadiation

!syntax inputs /UserObjects/FunctorGapFluxModelRadiation

!syntax children /UserObjects/FunctorGapFluxModelRadiation

!bibtex bibliography
