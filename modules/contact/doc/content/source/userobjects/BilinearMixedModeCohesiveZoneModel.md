# BilinearMixedModeCohesiveZoneModel

## Description

The `BilinearMixedModeCohesiveZoneModel` object computes a global traction vector
from a local traction vector that is calculated from a bilinear mixed mode cohesive
zone model.

This object inherits from [PenaltySimpleCohesiveZoneModel](/PenaltySimpleCohesiveZoneModel.md)
and can include penalty mortar mechanical contact.

Example of usage:

!listing modules/contact/test/tests/cohesive_zone_model/mortar_czm.i block=UserObjects/czm_uo

!syntax parameters /UserObjects/BilinearMixedModeCohesiveZoneModel

!syntax inputs /UserObjects/BilinearMixedModeCohesiveZoneModel

!syntax children /UserObjects/BilinearMixedModeCohesiveZoneModel
