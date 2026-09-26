# Shifted Boundary Method Module

The Shifted Boundary Method (SBM) enables the solution of partial differential equations
on complex geometries without requiring a boundary-fitted mesh. Instead of conforming
the background mesh to the true boundary, SBM constructs a nearby surrogate boundary
from existing element faces and transfers the boundary conditions to it. This approach
avoids cutting elements or regenerating the mesh while retaining the geometric
information required to accurately impose boundary conditions.

The same concept can be applied to internal interfaces. This extension, known as the
Shifted Interface Method (SIM), replaces a true material interface with a surrogate
interface. When applied to cohesive-zone modeling, SIM is referred to as the Shifted
Cohesive-Zone Method (SCZM).

SIM is not limited in principle to cohesive-zone problems. It can be applied to other
physics involving interface conditions, including thermal-contact problems,
fluid-interface problems, electromagnetics with dielectric interfaces, and mass
transport with membrane interfaces. This module currently provides tools for
constructing surrogate domains and interfaces, evaluating distances and normals from
MSH- or STL-based surface representations, and enforcing interface laws on
non-interface-fitted meshes.

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

## Shifted Interface Method

The Shifted Interface Method (SIM) imposes interface conditions on a surrogate interface
formed by faces of the background mesh rather than on the true interface. Because the
surrogate and true interfaces do not coincide, SIM accounts for this geometric
discrepancy when evaluating the interface conditions.

At each quadrature point on the surrogate interface, a distance vector maps the
quadrature point to its corresponding location on the true interface. A Taylor expansion
along this distance vector approximates the solution and other required quantities at the
true interface using values evaluated on the surrogate interface. For interface
conditions involving fluxes or tractions, the normal of the true interface is also used
to account for the difference in orientation between the true and surrogate interfaces.

This module currently demonstrates SIM through the Shifted Cohesive-Zone Method (SCZM)
for solid-mechanics interface problems. SCZM interface kernels enforce a cohesive
traction-separation law on the surrogate interface. The [Shifted Cohesive Zone Physics](syntax/Physics/SolidMechanics/ShiftedCohesiveZone/index.md)
creates the required user objects, materials, and interface kernels for this formulation.

## Citing

!! sbm-citation-start

The MOOSE implementation of the Shifted Cohesive-Zone Method (SCZM) is described by
Yang et al.:

```bibtex
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

The Shifted Boundary Method (SBM), which provides the foundation for this work, was
originally introduced by Main and Scovazzi:

```bibtex
@article{main2018shifted,
  title = {The shifted boundary method for embedded domain computations. Part I: Poisson and Stokes problems},
  author = {Main, Alex and Scovazzi, Guglielmo},
  journal = {Journal of Computational Physics},
  volume = {372},
  pages = {972--995},
  year = {2018},
  publisher = {Elsevier}
}
```

The extension of the shifted-boundary concept to internal interfaces, referred to as the
Shifted Interface Method (SIM), was introduced by Li et al.:

```bibtex
@article{li2020shifted,
  title = {The shifted interface method: a flexible approach to embedded interface computations},
  author = {Li, Kangan and Atallah, Nabil M and Main, G Alex and Scovazzi, Guglielmo},
  journal = {International Journal for Numerical Methods in Engineering},
  volume = {121},
  number = {3},
  pages = {492--518},
  year = {2020},
  publisher = {Wiley Online Library}
}
```

!! sbm-citation-end
