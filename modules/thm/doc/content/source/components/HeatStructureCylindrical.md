# HeatStructureCylindrical

This component is a 2D, axisymmetric domain on which the heat conduction equation
is solved.

## Usage

The initial temperature is given by the function parameter
[!param](/Components/HeatStructureCylindrical/initial_T).

If the domain has some inner radius, then this is specified with
[!param](/Components/HeatStructureCylindrical/inner_radius); otherwise, it is
assumed to be a solid cylinder. The domain may be divided up into any number (say, $n$) of regions in the
radial direction, which each get their own subdomain names and may use different
thermal properties. For example, if the domain were a fuel rod, two regions
could be used: the fuel itself and the cladding. The parameters
[!param](/Components/HeatStructureCylindrical/names),
[!param](/Components/HeatStructureCylindrical/widths),
[!param](/Components/HeatStructureCylindrical/n_part_elems), and
[!param](/Components/HeatStructureCylindrical/materials) are all lists of size
$n$, with entries corresponding to each radial region, ordered from the side
closest to the axis of the component. [!param](/Components/HeatStructureCylindrical/names) is a list of names
to assign to each region, which will be used to create subdomain names and to
refer to the regions in some objects. The radial width (thickness) of each region
is specified using [!param](/Components/HeatStructureCylindrical/widths),
and the number of radial elements in each region is given by
[!param](/Components/HeatStructureCylindrical/n_part_elems).

The parameter
[!param](/Components/HeatStructureCylindrical/materials) specifies the names of
[HeatStructureMaterials](HeatStructureMaterials/index.md) objects to use in
each region. Note that this parameter is optional; if omitted, the user must
create [Materials](Materials/index.md) supplying the material properties given
in [matprops_table].

### Variables and Material Properties

This component creates the variables listed in [vars_table].

!table id=vars_table caption=Variables on a heat structure domain.
| Variable | Symbol | Description |
| :- | :- | :- |
| `T_solid` | $T$ | Temperature \[K\] |

This component creates the material properties listed in [matprops_table].

!table id=matprops_table caption=Material properties on a heat structure domain.
| Material Property | Symbol | Description |
| :- | :- | :- |
| `density` | $\rho$ | Density \[kg/m$^3$\] |
| `specific_heat` | $c_p$ | Specific heat capacity \[J/(kg-K)\] |
| `thermal_conductivity` | $k$ | Thermal conductivity \[W/(m-K)\] |

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
| `<cname>:start` | The axial boundary of the end corresponding to the [!param](/Components/HeatStructureCylindrical/position) parameter |
| `<cname>:end` | The axial boundary of the end opposite to the [!param](/Components/HeatStructureCylindrical/position) parameter |
| `<cname>:<rname>:start` | The axial boundary of the end corresponding to the [!param](/Components/HeatStructureCylindrical/position) parameter in the radial region `<rname>` |
| `<cname>:<rname>:end` | The axial boundary of the end opposite to the [!param](/Components/HeatStructureCylindrical/position) parameter in the radial region `<rname>` |
| `<cname>:<rname1>:<rname2>` | The radial boundary between the radial regions `<rname1>` and `<rname2>` |
| `<cname>:<rname>:<aname1>:<aname2>` | The axial boundary in radial region `<rname>` between the axial regions `<aname1>` and `<aname2>` |

!syntax parameters /Components/HeatStructureCylindrical

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

!syntax inputs /Components/HeatStructureCylindrical

!syntax children /Components/HeatStructureCylindrical
