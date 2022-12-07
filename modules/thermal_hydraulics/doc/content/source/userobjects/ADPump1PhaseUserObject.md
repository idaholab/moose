# ADPump1PhaseUserObject

!syntax description /UserObjects/ADPump1PhaseUserObject

This user object is used with the [/Pump1Phase.md] component. It
computes the fluxes that are used in [/ADVolumeJunction1PhaseBC.md] for the two
connected flow channels and the residuals that are used in
[/ADVolumeJunctionAdvectionScalarKernel.md] for each of the pump's
scalar degrees of freedom. It also adds both a momentum and energy source term
due to the user supplied pump head.

!syntax parameters /UserObjects/ADPump1PhaseUserObject

!syntax inputs /UserObjects/ADPump1PhaseUserObject

!syntax children /UserObjects/ADPump1PhaseUserObject

!bibtex bibliography
