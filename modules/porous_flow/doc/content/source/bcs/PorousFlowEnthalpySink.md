# PorousFlowEnthalpySink

`PorousFlowEnthalpySink` implements a sink that adds heat energy corresponding to
adding fluid at a mass flux rate (computed by a function) at a specified temperature.

This object should be used in conjunction with (PorousFlowSink)[PorousFlowSink.md]
that uses the same `flux_function`, so that the correct amount of fluid is injected into the system.
Users are encouraged to utilize the (PorousFlowSinkBC)[PorousFlowSinkBC.md] to avoid making subtle mistakes.

Note that the fluid property object used by this boundary condition should be the same one that is
used in the computational domain where this object is located.

This BC can be used to model two situations.

- If [!param](/BCs/PorousFlowEnthalpySink/fluid_phase) is provided, then the injected enthalpy is calculated using the porepressure within the porous medium, and [!param](/BCs/PorousFlowEnthalpySink/T_in).  This corresponds to injecting at a specified rate and specified temperature which could potentially lead to changes in the porepressure (assuming the appropriate (PorousFlowSink)[PorousFlowSink.md] is also added).
- If [!param](/BCs/PorousFlowEnthalpySink/porepressure_var) is provided, then the injected enthalpy is calculated using this pressure (which may be an AuxVariable, for instance) and [!param](/BCs/PorousFlowEnthalpySink/T_in).  This corresponds to injecting a fluid at a specified rate, specified pressure and specified temperature.

For instance:

!listing modules/porous_flow/test/tests/sinks/s11.i block=BCs

!syntax parameters /BCs/PorousFlowEnthalpySink

!syntax inputs /BCs/PorousFlowEnthalpySink

!syntax children /BCs/PorousFlowEnthalpySink
