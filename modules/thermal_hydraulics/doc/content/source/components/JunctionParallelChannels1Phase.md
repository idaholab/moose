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
       alt=Diagram for the problem of a junction between a narrow inlet pipe and a wider outlet pipe.

It is assumed that the wall pressure is constant and equal to the inlet pressure. Then
the term $\int_{\Gamma_{\text{wall}}}{p\bf{n}}{d\Gamma}$ is reduced to the surface
that is in red in the above diagram.

If the outlet flow area is +greater+ than the inlet flow area, the wall pressure
 integral projected in the flow direction is:

!equation id=dp_out
\int_{\Gamma_{\text{wall}}}{p n_{\text{flow}}}{d\Gamma} = - p_{\text{wall}} A_{\text{wall}} =-p_{\text{in}} \left(A_{\text{out}}-A_{\text{in}}\right)

At an abrupt expansion the flow separates at the area step. Taking the pressure on the
step equal to the inlet pressure is the assumption of the Borda-Carnot model of a sudden
expansion [!cite](massey2006mechanics). With $p_{\text{wall}} = p_{\text{in}}$, the steady
momentum balance over the junction in the flow direction,

!equation id=momentum_expansion
p_{\text{in}} A_{\text{in}} + p_{\text{wall}} \left(A_{\text{out}} - A_{\text{in}}\right) - p_{\text{out}} A_{\text{out}} = \dot{m} \left(u_{\text{out}} - u_{\text{in}}\right),

where $\dot{m} = \rho_{\text{in}} u_{\text{in}} A_{\text{in}} = \rho_{\text{out}} u_{\text{out}} A_{\text{out}}$, gives the pressure rise

!equation id=dp_expansion
p_{\text{out}} - p_{\text{in}} = \frac{\dot{m} \left(u_{\text{in}} - u_{\text{out}}\right)}{A_{\text{out}}} = \rho_{\text{in}} u_{\text{in}}^2 \frac{A_{\text{in}}}{A_{\text{out}}} \left(1 - \frac{\rho_{\text{in}} A_{\text{in}}}{\rho_{\text{out}} A_{\text{out}}}\right).

For constant density, the change in total pressure $p + \frac{1}{2} \rho u^2$ across the junction is then

!equation id=dp0_expansion
\Delta p_0 = - \frac{1}{2} \rho \left(u_{\text{in}} - u_{\text{out}}\right)^2 = - \frac{1}{2} \rho u_{\text{in}}^2 \left(1 - \frac{A_{\text{in}}}{A_{\text{out}}}\right)^2,

which is the Borda-Carnot loss, with the loss coefficient $\left(1 - A_{\text{in}} / A_{\text{out}}\right)^2$ referred to the inlet velocity [!cite](idelchik2007handbook).

It follows from [!eqref](dp0_expansion) that $\Delta p_0$ is never positive, so total pressure does not increase across the junction, which has no energy source. If the wall pressure were instead taken equal to the outlet pressure, the same momentum balance would give [!eqref](dp_expansion) with $A_{\text{out}}$ replaced by $A_{\text{in}}$ in the denominator. The pressure rise would then be larger by the factor $A_{\text{out}} / A_{\text{in}}$, and $\Delta p_0$ would be positive.

If the outlet flow area is +smaller+ than the inlet flow area,
the wall pressure integral projected in the flow direction is:

!equation id=dp_in
\int_{\Gamma_{\text{wall}}}{p n_{\text{flow}}}{d\Gamma} =  p_{\text{wall}} A_{\text{wall}} =p_{\text{in}} \left(A_{\text{in}}-A_{\text{out}}\right)

### Form Loss

!template load file=volume_junction_1phase_formulation_formloss.md.template name=JunctionParallelChannels1Phase

!syntax inputs /Components/JunctionParallelChannels1Phase

!syntax children /Components/JunctionParallelChannels1Phase

!bibtex bibliography
