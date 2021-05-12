# PorousFlowSinkBC

This class adds a (PorousFlowSink)[PorousFlowSink.md] and a (PorousFlowEnthalpySink)[PorousFlowEnthalpySink.md] to model adding fluid at a mass flux rate at a specified temperature.

Users are required to specify:

- `boundary` - The list of boundary IDs from the mesh where this boundary condition applies
- either `fluid_phase` (the fluid phase whose porepressure is used to compute the injected enthalpy) or `porepressure_var` (an AuxVariable representing fluid pressure at which to compute the injected enthalpy)
- `T_in` - Inlet temperature
- `fp` - The name of the user object for fluid properties
- `flux_function` - The flux out of the medium

!listing modules/porous_flow/test/tests/sinks/s11_act.i block=/Modules/PorousFlow/BCs/left
