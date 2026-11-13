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

### Hydrostatic correction id=hydrostatic_correction

By default, the numerical flux at a junction face takes the cell average of the element next to
the junction and the solution of the junction center as if both were at the face. Under gravity,
this leaves a pressure error of $\rho g \Delta z / 4$ at the face of a fluid at rest, which sets
the fluid in motion. The parameter
[!param](/Components/VolumeJunction1Phase/use_hydrostatic_correction) moves the two solutions to
the face before the flux is computed and uses the face pressures of the junction in the wall
pressure term. See [modules/thermal_hydraulics/theory_manual/vace_model/volume_junction.md#hydrostatic_correction]
for the equations.

The junction takes the gravity vector from [!param](/Components/VolumeJunction1Phase/gravity_vector),
and the connected flow channels must have the same gravity vector and the same slope
reconstruction scheme ([!param](/Components/FlowChannel1Phase/rdg_slope_reconstruction)). The
option is off by default, for backward compatibility, and it is an experimental feature.

!alert tip title=Visualizing junction solutions
Since junction variables are located on the mesh, they can visualized alongside the other field variables, such as from pipes. In Paraview, the junction can be made more visible using the "point size" parameter.

## Implementation Notes

The junction variables are technically field variables that live on a block consisting of a single `NodeElem`. Originally, junction variables were scalar variables, but it was found that this was extremely costly due to the sparsity pattern requirements, thus making simulations involving large numbers of these components to be very slow to initialize.

!syntax parameters /Components/VolumeJunction1Phase

!syntax inputs /Components/VolumeJunction1Phase

!syntax children /Components/VolumeJunction1Phase

!bibtex bibliography
