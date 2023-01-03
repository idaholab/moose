# JunctionParallelChannels1Phase

This is a [volume junction](component_groups/volume_junction.md) that
connects an arbitrary number of parallel channels. If modeling elbows or
tees, [VolumeJunction1Phase.md] must be used instead.

This component can be used to model an abrupt flow area change, a plenum that is
divided into several channels, or a plenum that combines several channels into
one. [abrupt_dA] shows the channel configuration appropriate for this junction.

!media thermal_hydraulics/misc/Junction_parallel_channels.png
       id=abrupt_dA
       caption= Junction with parallel flow channels.
       style=width:50%;padding:20px;margin-left:auto;margin-right:auto

## Usage id=usage

!template load file=flow_junction_usage.md.template name=JunctionParallelChannels1Phase

!template load file=volume_junction_usage.md.template name=JunctionParallelChannels1Phase

!alert note title=Order-dependent connections
Several quantities in the form loss source terms given by [formloss_momentum] and [formloss_energy]
are taken from the first connection in [!param](/Components/JunctionParallelChannels1Phase/connections),
so using different connections in the first entry gives different results.

The parameter [!param](/Components/JunctionParallelChannels1Phase/A_ref) is the reference
cross-sectional area $A_\text{ref}$ used in [formloss_momentum] and [formloss_energy]. If it is
not provided, the cross-sectional area of the first connection in
[!param](/Components/JunctionParallelChannels1Phase/connections) is used.

A form loss coefficient $K$ may be specified using the parameter
[!param](/Components/JunctionParallelChannels1Phase/K).

!template load file=volume_junction_1phase_usage.md.template name=JunctionParallelChannels1Phase

!syntax parameters /Components/JunctionParallelChannels1Phase

## Formulation

The junction equations are derived by integrating the conservation equations
over the junction volume. This formulation assumes that all connected channels
have the same flow direction. For the momentum equation, the term
$\int_{\Gamma_{\text{wall}}}{p\bf{n}}{d\Gamma}$ must be evaluated. For this
junction, it is assumed that there is an abrupt flow area change within the
junction. For the case where the inlet flow area is smaller than the outlet flow
area, the geometry of the junction is shown below.

!media thermal_hydraulics/misc/parallel_junction_geometry.png
       style=width:40%;display:block;margin-left:auto;margin-right:auto

It is assumed that the wall pressure is:

- constant and equal to the inlet pressure upstream of the area change.
- constant and equal to the outlet pressure downstream the area change.

Then the term $\int_{\Gamma_{\text{wall}}}{p\bf{n}}{d\Gamma}$
is reduced to the surface that is in red in the above diagram.

If the outlet flow area is +greater+ than the inlet flow area, the wall pressure
 integral projected in the flow direction is:

!equation id=dp_out
\int_{\Gamma_{\text{wall}}}{p n_{\text{flow}}}{d\Gamma} = - p_{\text{wall}} A_{\text{wall}} =-p_{\text{out}} \left(A_{\text{out}}-A_{\text{in}}\right)

If the outlet flow area is +smaller+ than the inlet flow area,
the wall pressure integral projected in the flow direction is:

!equation id=dp_in
\int_{\Gamma_{\text{wall}}}{p n_{\text{flow}}}{d\Gamma} =  p_{\text{wall}} A_{\text{wall}} =p_{\text{in}} \left(A_{\text{in}}-A_{\text{out}}\right)

### Form Loss

!template load file=volume_junction_1phase_formulation_formloss.md.template name=JunctionParallelChannels1Phase

!syntax inputs /Components/JunctionParallelChannels1Phase

!syntax children /Components/JunctionParallelChannels1Phase
