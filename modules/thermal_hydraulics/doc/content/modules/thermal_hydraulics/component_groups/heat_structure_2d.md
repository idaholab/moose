# 2D Heat Structures

The [heat structures](thermal_hydraulics/component_groups/heat_structure.md)
that are 2D share parameters that specify how to generate the 2D mesh.

The following components are 2D heat structures:

- [HeatStructureCylindrical.md]
- [HeatStructurePlate.md]

## Usage

The domain may be divided up into any number (say, $n$) of regions in the
radial direction, which each get their own subdomain names and may use different
thermal properties. For example, if the domain were a fuel rod, two regions
could be used: the fuel itself and the cladding. The parameters
`names`, `widths`, `n_part_elems`, and `materials` are all lists of size
$n$, with entries corresponding to each radial region, ordered from the side
closest to the axis of the component. `names` is a list of names
to assign to each region, which will be used to create subdomain names and to
refer to the regions in some objects. The radial width (thickness) of each region
is specified using `widths`, and the number of radial elements in each region is given by
`n_part_elems`.

The parameter `materials` specifies the names of
[HeatStructureMaterials](HeatStructureMaterials/index.md) objects to use in
each region. Note that this parameter is optional; if omitted, the user must
create [Materials](Materials/index.md) supplying the material properties given
in [matprops_table].

### Variables and Material Properties

If the `materials` parameter is supplied, this component creates the material
properties in [matprops_table]:

!table id=matprops_table caption=Material properties on a heat structure domain.
| Material Property | Symbol | Description |
| :- | :- | :- |
| `density` | $\rho$ | Density \[kg/m$^3$\] |
| `specific_heat` | $c_p$ | Specific heat capacity \[J/(kg-K)\] |
| `thermal_conductivity` | $k$ | Thermal conductivity \[W/(m-K)\] |

If the `materials` parameter is omitted, the user is responsible for creating
these material properties on all of the blocks on the heat structure.

### Mesh Entities

This component creates the blocks listed in [blocks_table], where `<cname>` is
the user-given name of the component.

!table id=blocks_table caption=Blocks on a heat structure domain.
| Block | Description |
| :- | :- |
| `<cname>:<rname>` | The radial region `<rname>` |

This component creates the boundaries listed in [boundaries_table].

!table id=boundaries_table caption=Boundaries on a heat structure domain.
| Boundary | Description |
| :- | :- |
| `<cname>:inner` | The innermost radial boundary |
| `<cname>:outer` | The outermost radial boundary |
| `<cname>:<aname>:inner` | The innermost radial boundary in the axial section `<aname>` |
| `<cname>:<aname>:outer` | The outermost radial boundary in the axial section `<aname>` |
| `<cname>:start` | The axial boundary of the end corresponding to the `position` parameter |
| `<cname>:end` | The axial boundary of the end opposite to the `position` parameter |
| `<cname>:<rname>:start` | The axial boundary of the end corresponding to the `position` parameter in the radial region `<rname>` |
| `<cname>:<rname>:end` | The axial boundary of the end opposite to the `position` parameter in the radial region `<rname>` |
| `<cname>:<rname1>:<rname2>` | The radial boundary between the radial regions `<rname1>` and `<rname2>` |
| `<cname>:<rname>:<aname1>:<aname2>` | The axial boundary in radial region `<rname>` between the axial regions `<aname1>` and `<aname2>` |

## Formulation

The heat conduction equation is the following:
\begin{equation}
  \rho c_p \pd{T}{t} - \nabla \cdot (k \nabla T) = 0 \eqc
\end{equation}
where

- $\rho$ is density,
- $c_p$ is specific heat capacity,
- $k$ is thermal conductivity, and
- $T$ is temperature.

Multiplying by a test function $\phi_i$ and integrating by parts over the domain
$\Omega$ gives
\begin{equation}
  \pr{\rho c_p \pd{T}{t}, \phi_i}_\Omega + \pr{k \nabla T, \nabla\phi_i}_\Omega
    - \left\langle k \nabla T, \phi_i\mathbf{n}\right\rangle_{\partial\Omega} = 0 \eqc
\end{equation}
where $\partial\Omega$ is the boundary of the domain $\Omega$.
