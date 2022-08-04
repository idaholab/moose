# FunctorGapFluxModelConduction

!syntax description /UserObjects/FunctorGapFluxModelConduction

## Description

`FunctorGapFluxModelConduction` implements the same equations as
[GapFluxModelConduction.md], however, it uses the functor system to evaluate
needed quantities on-the-fly. The functor system is leveraged heavily by MOOSE's
finite volume discretizations. To use pre-initialized data, which is the
tradition for finite element discretizations, the [GapFluxModelConduction.md]
object may be the more appropriate object to use.

## Example Input File Syntax

!listing modules/heat_conduction/test/tests/gap_heat_transfer_mortar/fv_modular_gap_heat_transfer_mortar_radiation_conduction.i block=UserObjects/conduction

`FunctorGapFluxModelConduction` must be used in conjunction with the modular gap conductance
constraint as shown below:

!listing modules/heat_conduction/test/tests/gap_heat_transfer_mortar/fv_modular_gap_heat_transfer_mortar_radiation_conduction.i block=Constraints/ced

!syntax parameters /UserObjects/FunctorGapFluxModelConduction

!syntax inputs /UserObjects/FunctorGapFluxModelConduction

!syntax children /UserObjects/FunctorGapFluxModelConduction

!bibtex bibliography
