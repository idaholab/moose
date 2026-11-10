# PorousFlowSinkBC

This class adds a [PorousFlowSink](PorousFlowSink.md) and a [PorousFlowEnthalpySink](PorousFlowEnthalpySink.md) to model adding fluid at a mass flux rate and specified temperature.

!alert note
Despite its name, PorousFlowSinkBC adds fluid rather than removing it.  The term "Sink" is used solely to maintain consistency with other PorousFlow objects.

This BC can be used to model two situations, both of which involve injecting fluid at a specified rate and specified temperature:

- If [!param](/BCs/PorousFlowEnthalpySink/porepressure_var) is provided, then the injected enthalpy is calculated using this pressure (which may be an AuxVariable, for instance) and [!param](/BCs/PorousFlowEnthalpySink/T_in).  This corresponds to injecting a fluid at a specified rate, specified pressure and specified temperature.
- If [!param](/BCs/PorousFlowEnthalpySink/fluid_phase) is provided, then the injected enthalpy is calculated using the porepressure within the porous medium, and [!param](/BCs/PorousFlowEnthalpySink/T_in).

In both cases `flux_function` must be negative. Its magnitude gives the fluid mass flux into the model (kg.m$^{-2}$.s$^{-1}$).

!listing modules/porous_flow/test/tests/sinks/s11_act.i block=/Modules/PorousFlow/BCs/left

!syntax parameters /Modules/PorousFlow/BCs/PorousFlowSinkBC

!syntax inputs /Modules/PorousFlow/BCs/PorousFlowSinkBC

!syntax children /Modules/PorousFlow/BCs/PorousFlowSinkBC
