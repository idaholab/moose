# Pump1PhaseUserObject

!syntax description /UserObjects/Pump1PhaseUserObject

This user object is used with the [/Pump1Phase.md] component. It
computes the fluxes that are used in [/VolumeJunction1PhaseBC.md] for the two
connected flow channels and the residuals that are used in
[/VolumeJunctionAdvectionScalarKernel.md] for each of the pump's
scalar degrees of freedom. It also adds both a momentum and energy source term
due to the user supplied pump head.

!syntax parameters /UserObjects/Pump1PhaseUserObject

!syntax inputs /UserObjects/Pump1PhaseUserObject

!syntax children /UserObjects/Pump1PhaseUserObject

!bibtex bibliography
