# FlowChannel1PhaseWithAddedMass

This component inherits from [FlowChannel1Phase.md] but adds a mass imbalance
source term via a Postprocessor defined via `mass_imbalance_source_pp`.

This component is primarily used for domain overlapping coupling
with higher-fidelity thermal-hydralics components.

!syntax parameters /Components/FlowChannel1PhaseWithAddedMass

!syntax inputs /Components/FlowChannel1PhaseWithAddedMass

!syntax children /Components/FlowChannel1PhaseWithAddedMass
