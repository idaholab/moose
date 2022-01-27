# JunctionParallelChannels1Phase

This is a single-phase [flow junction](component_groups/flow_junction.md) to
connect an arbitrary number of parallel channels. This component can be used to
model an abrupt flow area change, a plenum that is divided into several channels
or a plenum that combines several channels into one. [abrupt_dA] shows the
channel configuration appropriate for this junction. The more general
[volume junction](source/components/VolumeJunction1Phase.md) formulation is
recommended to model elbows or tees.

!media images/Junction_parallel_channels.png
       id=abrupt_dA
       caption= Junction with parallel flow channels.
       style=width:50%;padding:20px;

## Usage

The user specifies the following parameters:

- `connections`: the flow channel boundaries that are connected to the junction.
- `position`: the position of the junction. The position is used for plotting purposes only.
- `volume`: the volume of the junction.
- `K` and `A_ref`: A form factor `K` can be added to the junction. It is based on the user-specified area `A_ref`. Both parameters must be supplied to apply the form losses.


!syntax parameters /Components/JunctionParallelChannels1Phase

!syntax inputs /Components/JunctionParallelChannels1Phase

!syntax children /Components/JunctionParallelChannels1Phase

## Formulation

The junction equations are derived by integrating the conservation equations
over the junction volume. This formulation assumes that all connected channels
have the same flow direction. For the momentum equation, the term
$\int_{\Gamma_{\text{wall}}}{p\bf{n}}{d\Gamma}$ must be evaluated. For this
junction, it is assumed that there is an abrupt flow area change within the
junction. For the case where the inlet flow area is smaller than the outlet flow
area, the geometry of the junction is shown below.

!media images/parallel_junction_geometry.png  style=width:40%;

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
