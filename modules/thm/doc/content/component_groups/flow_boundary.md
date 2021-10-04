# Flow Boundaries

Flow boundary components are attached to ends of
[flow channel components](component_groups/flow_channel.md) to provide boundary conditions.

## Usage id=usage

A flow boundary component connects to a flow channel end using the `input`
parameter. The value of this parameter corresponds to the
[boundary name corresponding to a specific end of a flow channel](component_groups/flow_channel.md#spatial_domain). The format for the `input` parameter is
`flow_channel_name:in` or `flow_channel_name:out`, where `flow_channel_name`
is the name of the connected flow channel, and `in` or `out` depends on which
end is being connected. The `in` side denotes the "starting" side of the flow
channel, which resides at the location specified by the flow channel's `position`
parameter, and the `out` side is the opposite end.

## Formulation id=formulation

Flow boundary formulations differ based on how the boundary flux $\mathbf{F}_\text{b}$
is computed.

### Direct Flux Computation id=direct_flux

In this case, the boundary flux is computed directly from the continuous flux
function $\mathbf{f}(\mathbf{u})$ using some solution vector $\mathbf{U}_\text{b}$,
which is determined by the boundary condition and is in general a combination of
the interior solution and exterior boundary data:
\begin{equation}
  \mathbf{F}_\text{b} = \mathcal{f}(\mathbf{U}_b) \eqp
\end{equation}

### Ghost Cell Flux Computation id=ghostcell_flux

In this case, the boundary flux is computed using a ghost cell approach. That
is, the flux $\mathbf{F}_\text{b}$ corresponding to the boundary is computed
using a numerical flux function $\mathcal{F}(\mathbf{U}_L, \mathbf{U}_R)$,
taking the interior solution $\mathbf{U}_i$ and ghost cell solution
$\mathbf{U}_\text{ghost}$ as inputs:
\begin{equation}
  \mathbf{F}_\text{b} = \mathcal{F}(\mathbf{U}_i, \mathbf{U}_\text{ghost}) \eqc
\end{equation}
in this case for the "right" boundary of the flow channel.

Each boundary condition is defined by its definition of $\mathbf{U}_\text{ghost}$
and in general is formed by taking some values from the interior solution and
some from external data.
