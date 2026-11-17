# SubdomainInterceptedGenerator

!syntax description /Mesh/SubdomainInterceptedGenerator

## Overview

The `SubdomainInterceptedGenerator` classifies and assigns subdomain IDs to elements in an input mesh based on an implicit interface defined by an analytical signed distance function expression $\phi(x, y, z)$.

This generator evaluates the signed distance function across the mesh to assign elements to:
- An **inside** subdomain ([!param](/Mesh/SubdomainInterceptedGenerator/subdomain_id_inside)),
- An **outside** subdomain ([!param](/Mesh/SubdomainInterceptedGenerator/subdomain_id_outside)), and
- Optionally, a dedicated **intercepted** subdomain ([!param](/Mesh/SubdomainInterceptedGenerator/subdomain_id_intercepted)) for elements cut by the interface.

## Classification Method

The interface is defined by the level-set condition:
\begin{equation*}
\phi(x, y, z) = \text{threshold}
\end{equation*}
where $\text{threshold}$ is set by [!param](/Mesh/SubdomainInterceptedGenerator/threshold) (default is $0.0$).

### Domain Orientation

The orientation of the inside domain relative to the signed distance function is controlled by [!param](/Mesh/SubdomainInterceptedGenerator/is_domain_inside_surface):
- **`is_domain_inside_surface = true`**: Follows the standard convention where negative signed distance values represent the interior of an enclosed region. Points satisfying $\phi(x, y, z) < \text{threshold}$ are categorized as inside the domain, and points satisfying $\phi(x, y, z) > \text{threshold}$ are categorized as outside.
- **`is_domain_inside_surface = false`**: Inverts the convention. Points satisfying $\phi(x, y, z) > \text{threshold}$ are categorized as inside the domain, and points satisfying $\phi(x, y, z) < \text{threshold}$ are categorized as outside.

### Element Occupancy and Intercepted Elements

Each element in the input mesh is evaluated at its vertices (nodes) and sampled across its volume using a Gauss quadrature rule of order [!param](/Mesh/SubdomainInterceptedGenerator/qrule_order) (default is 9). The generator measures:
1. Whether all element nodes lie inside the domain (`all_nodes_in_domain`).
2. Whether all element nodes lie outside the domain (`all_nodes_outside_domain`).
3. The fraction of the element volume occupied by the inside region (`fraction_of_element_in_domain`), evaluated via numerical quadrature.

Elements are then categorized as follows:
- **Fully inside**: When all nodes lie inside and `fraction_of_element_in_domain == 1.0`, the element is assigned [!param](/Mesh/SubdomainInterceptedGenerator/subdomain_id_inside).
- **Fully outside**: When all nodes lie outside and `fraction_of_element_in_domain == 0.0`, the element is assigned [!param](/Mesh/SubdomainInterceptedGenerator/subdomain_id_outside).
- **Intercepted (cut) elements**: Elements that contain both inside and outside points are cut by the implicit boundary:
  - If [!param](/Mesh/SubdomainInterceptedGenerator/mark_intercepted) is `true`, these elements are assigned a dedicated subdomain ID specified by [!param](/Mesh/SubdomainInterceptedGenerator/subdomain_id_intercepted).
  - If [!param](/Mesh/SubdomainInterceptedGenerator/mark_intercepted) is `false` (the default), the element is resolved to either inside or outside based on the volume fraction threshold [!param](/Mesh/SubdomainInterceptedGenerator/lambda) (default is $0.5$). If the fraction of the element volume outside the domain exceeds $\lambda$, the element is assigned to outside; otherwise, it is assigned to inside.

## Multi-Geometry Workflows

When constructing domains with multiple geometric features, inclusions, or composite shapes, multiple `SubdomainInterceptedGenerator` instances can be chained in sequence. To prevent subsequent generators from unintentionally overwriting previously assigned subdomains, several parameters are available:

- [!param](/Mesh/SubdomainInterceptedGenerator/multi_geo): Enables multi-geometry retention checks.
- [!param](/Mesh/SubdomainInterceptedGenerator/keep_inside_as_inside): When `multi_geo = true`, any element whose existing subdomain ID already matches [!param](/Mesh/SubdomainInterceptedGenerator/subdomain_id_inside) is skipped and preserved, preventing later generators from overwriting it.
- [!param](/Mesh/SubdomainInterceptedGenerator/keep_outside_as_outside): When `multi_geo = true`, any element whose existing subdomain ID already matches [!param](/Mesh/SubdomainInterceptedGenerator/subdomain_id_outside) is skipped and preserved.
- [!param](/Mesh/SubdomainInterceptedGenerator/modify_inside_only): When `true`, only elements categorized as inside receive [!param](/Mesh/SubdomainInterceptedGenerator/subdomain_id_inside). Elements categorized as outside retain their pre-existing subdomain IDs.
- [!param](/Mesh/SubdomainInterceptedGenerator/modify_outside_only): When `true`, only elements categorized as outside receive [!param](/Mesh/SubdomainInterceptedGenerator/subdomain_id_outside). Elements categorized as inside retain their pre-existing subdomain IDs.

## Example Input File Syntax

### Inclined Interface Partitioning

The following example demonstrates using `SubdomainInterceptedGenerator` to partition a 2D mesh across an inclined plane into subdomains 1 and 2, with `is_domain_inside_surface = true`:

!listing modules/shifted_boundary_method/test/tests/shifted_cohesive_zone/angled_sczm_common.i start=[Mesh] end=[UserObjects] include-end=false

### Inverted Domain Orientation

When `is_domain_inside_surface = false`, points on the positive side of the signed distance function are categorized into the inside subdomain:

!listing modules/shifted_boundary_method/test/tests/shifted_cohesive_zone/sczm_triangle_mms.i start=[subdomain_intercepted] end=[break] include-end=false

!syntax parameters /Mesh/SubdomainInterceptedGenerator

!syntax inputs /Mesh/SubdomainInterceptedGenerator

!syntax children /Mesh/SubdomainInterceptedGenerator
