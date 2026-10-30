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

The junction and the connected flow channels exchange fluxes through the junction faces, which
are the ends of the flow channels. The flux at a face is computed by the numerical flux of the
flow channel from two solutions: the solution of the flow channel and the solution of the
junction. By default, the flow channel solution is the cell average of the element next to the
junction, which is the solution at the cell center, half a cell away from the face, and the
junction solution is the one of the junction center. Under gravity, the pressure at the cell
center differs from the pressure at the face by $\rho g \Delta z / 2$, and the numerical flux
treats the two solutions as if they were at the same distance from the face. For a fluid at
rest, the pressure at the face is then off by $\rho g \Delta z / 4$, and the fluid moves until
the momentum flux of the spurious velocity balances this error.

The parameter [!param](/Components/VolumeJunction1Phase/use_hydrostatic_correction) brings the
two solutions to the face before the flux is computed:

- the flow channel solution at the face is the slope reconstruction of the element next to the
  junction, with the same scheme as in the interior of the channel
  ([!param](/Components/FlowChannel1Phase/rdg_slope_reconstruction)). This element has only one
  neighbor, so the junction is used as the other one: it is placed at the mirror point of the
  cell center across the face, and its solution is moved to that point along the hydrostatic
  balance, so that the two one-sided slopes agree for a fluid at rest;
- the junction solution is moved from the junction center $\mathbf{x}_J$, given by
  [!param](/Components/VolumeJunction1Phase/position), to each face $\mathbf{x}_i$ with the
  density of the junction, $p_{J,i} = p_J + \rho_J \, \mathbf{g} \cdot (\mathbf{x}_i - \mathbf{x}_J)$,
  keeping the density and the velocity, as in the ghost cell of the pressure boundary condition.

The wall pressure term of the momentum equation, $p_J \sum_i \mathbf{n}_i A_i$, uses the
pressures $p_{J,i}$ at the faces. No gravity term is added to the momentum equation of the
junction: with a hydrostatic pressure on the walls of the junction, the gravity force on the
junction volume cancels with the wall pressure integral, and only the face pressures remain. With
this option, a fluid at rest with a linear pressure profile in each element stays at rest, for any
position of the faces relative to the junction center. With
[!param](/Components/FlowChannel1Phase/rdg_slope_reconstruction) set to `none`, the flow channel
solution at the face is the cell average, as at the flow channel boundaries, and only the junction
side is corrected.

The junction takes the gravity vector from [!param](/Components/VolumeJunction1Phase/gravity_vector),
usually set in `GlobalParams`, and the connected flow channels must have the same gravity vector
and the same slope reconstruction scheme. The option is off by default, for backward
compatibility, and it is an experimental feature.

!alert tip title=Visualizing junction solutions
Since junction variables are located on the mesh, they can visualized alongside the other field variables, such as from pipes. In Paraview, the junction can be made more visible using the "point size" parameter.

## Implementation Notes

The junction variables are technically field variables that live on a block consisting of a single `NodeElem`. Originally, junction variables were scalar variables, but it was found that this was extremely costly due to the sparsity pattern requirements, thus making simulations involving large numbers of these components to be very slow to initialize.

!syntax parameters /Components/VolumeJunction1Phase

!syntax inputs /Components/VolumeJunction1Phase

!syntax children /Components/VolumeJunction1Phase

!bibtex bibliography
