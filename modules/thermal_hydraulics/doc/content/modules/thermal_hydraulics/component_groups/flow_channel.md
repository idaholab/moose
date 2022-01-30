# Flow Channels

## Usage

### Spatial Domain id=spatial_domain

A 1-D flow channel domain is defined by defining the line segment in 3-D space.
The line segment is defined with a "start" point $\mathbf{x}_\text{start}$,
corresponding to either end, the direction $\mathbf{d}$ to the other end, and
the distance in that direction, $L$. Thus the other end of the line segment is
\begin{equation}
  \mathbf{x}_\text{end} = \mathbf{x}_\text{start} + L \mathbf{d} \eqp
\end{equation}
These quantities are defined using the following parameters:

- `position`: the "start" point $\mathbf{x}_\text{start}$,
- `orientation`: the direction $\mathbf{d}$ (which gets automatically normalized), and
- `length`: the length(s) (see [#mesh] for explanation) that sum to $L$.

Note that the parameter `orientation` can only be used to specify a single
direction and thus cannot be used to specify bends in a flow channel.

The name given to the flow channel component, say, `<flow_channel_name>`, is
used internally to create a subdomain (also called a "block") name. Many MOOSE
objects such as post-processors use a parameter `block` to restrict their operation
to a list of subdomains, so the component name `<flow_channel_name>` can be used
in those places.

Additionally, two `boundary` names are created with the following convention:

- `<flow_channel_name>:in` corresponds to the "start" end of `<flow_channel_name>`,
  which corresponds to the parameter `position` of `<flow_channel_name>`, and
- `<flow_channel_name>:out` corresponds to the "end" end of `<flow_channel_name>`.

Each end of a flow channel must be connected to either a [boundary](component_groups/flow_boundary.md) or a [junction](component_groups/flow_junction.md).

### Mesh id=mesh

The most basic mesh specification is given by a single value for the parameters
`length` and `n_elems`, which correspond to the length of the channel and number
of uniformly-sized elements to use. For example, the following parameters would
specify a flow channel of length $L = 50$ m, divided into 100 elements (each
with width 0.5 m):

```
length = 50
n_elems = 100
```

The `length` and `n_elems` parameters can also be supplied with multiple values.
Multiple values correspond to splitting the length into segments that can have
different element sizes. However, within each segment, the discretization is
assumed uniform. The numbers of elements in each segment are specified with the
parameter `n_elems`, with entries corresponding to the entries in `length`. For
example, the following would also specify a flow channel of length $L = 50$ m
with 100 total elements, but in this case the first 10 m have 40 elements of
size 0.25 m, whereas the last 40 m have 60 elements of size $0.\bar{6}$ m.

```
length = '10 40'
n_elems = '40 60'
```

When using more than one entry in the `length` and `n_elems` parameters, the
parameter `axial_region_names` is used to provide names for each entry. These
get used to create blocks with the format `<component_name>:<region_name>`,
where `<component_name>` is the name of the component and `<region_name>` is
an entry in the `axial_region_names` parameter.
