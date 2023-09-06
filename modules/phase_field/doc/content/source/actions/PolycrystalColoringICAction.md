# PolycrystalColoringICAction

!syntax description /ICs/PolycrystalICs/PolycrystalColoringIC/PolycrystalColoringICAction

## Overview

This simplifies the syntax for creating polycrystal ICs. It automatically creates an IC for each variable used in the grain growth model. It works for both the traditional phase field grain growth model and the linearized interface grain growth model. Depending on the value of `linearized_interface`, two different ICs are used for each variable:

- For `linearized_interface = false`, [PolycrystalColoringIC](/PolycrystalColoringIC.md) is used.
- For `linearized_interface = true`, [PolycrystalColoringICLinearizedInterface](/PolycrystalColoringICLinearizedInterface.md) is used.

Both of these ICs require a `UserObject` to define the actual polycrystal geometry. See [Polycrystal ICs](/PolycrystalICs.md) for more information and a full list of the options.


## Example Input File Syntax

For `linearized_interface = false`

!listing modules/phase_field/test/tests/grain_tracker_test/distributed_poly_ic.i block=GlobalParams

!listing modules/phase_field/test/tests/grain_tracker_test/distributed_poly_ic.i block=ICs

For `linearized_interface = true`

!listing modules/phase_field/test/tests/grain_growth_w_linearized_interface/voronoi_linearized_interface.i block=GlobalParams

!listing modules/phase_field/test/tests/grain_growth_w_linearized_interface/voronoi_linearized_interface.i block=ICs

!syntax parameters /ICs/PolycrystalICs/PolycrystalColoringIC/PolycrystalColoringICAction

!syntax inputs /ICs/PolycrystalICs/PolycrystalColoringIC/PolycrystalColoringICAction

!syntax children /ICs/PolycrystalICs/PolycrystalColoringIC/PolycrystalColoringICAction
