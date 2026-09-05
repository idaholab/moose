# Shifted Boundary Method Module

The Shifted Boundary Method (SBM) enables the solution of partial differential equations
on complex geometries without requiring a boundary-fitted mesh. Instead of conforming
the background mesh to the true boundary, SBM constructs a nearby surrogate boundary
from existing element faces and transfers the boundary conditions to it. This approach
avoids cutting elements or regenerating the mesh while retaining the geometric
information required to accurately impose boundary conditions.

The same concept can be applied to internal interfaces. This extension, known as the
Shifted Interface Method (SIM), replaces a true material interface with a surrogate
interface. This module applies SIM to solid-mechanics interface problems through the
Shifted Cohesive-Zone Method (SCZM). It provides tools for constructing surrogate
domains and interfaces, evaluating distances and normals from MSH- or STL-based surface
representations, and enforcing cohesive-zone laws on non-interface-fitted meshes.

The SCZM workflow has three main components.

## Surrogate domains and interfaces

Each surrogate domain or grain consists of complete background-mesh elements. The
[`PointInSubdomainCheckUO`](source/userobjects/PointInSubdomainCheckUO.md) performs
point-in-polyhedron tests against closed surface meshes, and the
[`SubdomainElementModifier`](source/meshmodifiers/SubdomainElementModifier.md) uses
those tests to assign elements to surrogate subdomains. Faces separating active and
inactive elements form a surrogate boundary. For polycrystalline finite element models,
faces separating elements assigned to different grains form surrogate grain interfaces.

## Distance vectors and true normals

SBM and SIM transfer conditions from the true boundary or interface to its surrogate.
The module therefore computes a distance vector from each surrogate quadrature point to
its projection on the true geometry. This vector enters a Taylor expansion that accounts
for the offset between the two geometries. For flux or traction conditions, the normal at
the projected point on the true geometry is also required to account for the difference
between the true and surrogate surface measures.

The distance and normal data are provided by objects such as
[`ShortestDistanceToSurface`](source/userobjects/ShortestDistanceToSurface.md) and
[`BoundaryShortestDistanceToSurface`](source/userobjects/BoundaryShortestDistanceToSurface.md).

## Shifted Cohesive-Zone Method

SCZM combines the surrogate interface with the distance and normal information described
above. Interface kernels enforce the cohesive traction-separation law on the surrogate
interface. Their weak form uses the distance vector to evaluate the shifted displacement
jump and the true normal to correct the traction contribution for the geometric mismatch.
The [`ShiftedCohesiveZoneAction`](source/actions/ShiftedCohesiveZoneAction.md) creates the
required user objects, materials, and interface kernels for this formulation.

## Citing

!! sbm-citation-start

The following paper describes the formulation, implementation, verification, and
applications of the Shifted Cohesive-Zone Method:

```
@article{yang2026shifted,
  author = {Cheng-Hau Yang and Mark C. Messner and Tianchen Hu},
  title = {A Shifted Cohesive-Zone Method for Non-Interface-Fitted Meshes with Applications to Crystal Plasticity},
  journal = {Computer Methods in Applied Mechanics and Engineering},
  publisher = {Elsevier},
  volume = {461},
  pages = {119259},
  year = {2026},
  issn = {0045-7825},
  doi = {https://doi.org/10.1016/j.cma.2026.119259},
  url = {https://www.sciencedirect.com/science/article/pii/S0045782526005323}
}
```

!! sbm-citation-end
