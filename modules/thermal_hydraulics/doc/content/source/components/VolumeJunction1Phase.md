# VolumeJunction1Phase

This is a [flow junction](component_groups/flow_junction.md) that has a volume
and can connect 2 or more [FlowChannel1Phase.md] components in any orientation.

## Formulation

See [modules/thermal_hydraulics/theory_manual/vace_model/volume_junction.md] for
the theoretical formulation.

### Form Losses

!template load file=volume_junction_1phase_formulation_formloss.md.template name=VolumeJunction1Phase

## Usage id=usage

!template load file=flow_junction_usage.md.template name=VolumeJunction1Phase

A form loss coefficient $K$ may be specified using the parameter
[!param](/Components/VolumeJunction1Phase/K).
The parameter [!param](/Components/VolumeJunction1Phase/A_ref) is the reference
cross-sectional area $A_\text{ref}$ used in [formloss_momentum] and [formloss_energy]. If it is
not provided, the cross-sectional area of the first connection in
[!param](/Components/VolumeJunction1Phase/connections) is used.

!alert note title=Connection order matters when using form loss
The order of connections in [!param](/Components/VolumeJunction1Phase/connections)
has an impact when using form loss, since some quantities in [formloss_momentum] and [formloss_energy]
are taken from the *first* connection.

!template load file=volume_junction_1phase_usage.md.template name=VolumeJunction1Phase

!syntax parameters /Components/VolumeJunction1Phase

!syntax inputs /Components/VolumeJunction1Phase

!syntax children /Components/VolumeJunction1Phase

!bibtex bibliography
