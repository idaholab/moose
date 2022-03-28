# RelationshipManager

The RelationshipManager system is for making MOOSE aware of extra geometric,
algebraic, or coupling information needed to perform a calculation. This is an
extension of the "stencil" concept when working with a structured mesh and
performing calculations where you need neighboring information. Relationship
managers (RMs) come in three flavors: geometric, algebraic, and coupling.  For
discerning between the three types, we can think of three levels of "element
dependencies". An element K1 has a coupling dependency on element K2 if the dofs
on K1 might need sparsity pattern entries for dofs on K2.  An element K1 has an
algebraic dependency on K2 if a processor which owns K1 might need to examine
the solution dof values on K2.  An element K1 has a geometric dependency on K2
if a processor which owns K1 might need to examine the geometry of K2. For any
element K, we could call the set of coupling-ghosted ("coupled") elements C(K),
call the set of algebraic-ghosted ("evaluable") elements E(K), and call the set
of geometry-ghosted ("ghosted") elements G(K). It should be safe to assume that,
for any element K, C(K) implies E(K) implies G(K), meaning that a requirement
for coupling ghosting means that algebraic and geometric ghosting will be
required as well. As a more concrete example, it will be impossible to
investigate the degrees of freedom on an element in the `libMesh::DofMap` if
that element has been deleted from the current process, e.g. if we forgot to
geometrically ghost the element. Hence geometrically ghosted elements must be a
super-set of algebraically ghosted elements which must be a super-set of
coupling-ghosted elements.

The RM system is fully pluggable like other MOOSE systems but it not intended to
be exposed in the input file in any way. Instead, individual objects that
require extra information should "register" an appropriate RM right in its
`validParams` function and have MOOSE add the appropriate object to the
simulation at the proper time. It is possible for a single RM to define
geometric, algebraic, and coupling-ghosting, e.g like the RM defined for
`DGKernelBase` derived-objects:

```c++
params.addRelationshipManager("ElementSideNeighborLayers",
                              Moose::RelationshipManagerType::GEOMETRIC |
                                  Moose::RelationshipManagerType::ALGEBRAIC |
                                  Moose::RelationshipManagerType::COUPLING);
```

An application developer should always respect the superset relationships
discussed in the introductory paragraph, e.g. the following relationship
manager additions would provide a valid ghosting set-up:

```c++
params.addRelationshipManager(
    "ElementSideNeighborLayers",
    Moose::RelationshipManagerType::GEOMETRIC,
    [](const InputParameters &, InputParameters & rm_params) {
      rm_params.set<unsigned short>("layers") = 3;
    }
);
params.addRelationshipManager(
    "ElementSideNeighborLayers",
    Moose::RelationshipManagerType::ALGEBRAIC,
    [](const InputParameters &, InputParameters & rm_params) {
      rm_params.set<unsigned short>("layers") = 2;
    }
);
params.addRelationshipManager(
    "ElementSideNeighborLayers",
    Moose::RelationshipManagerType::COUPLING,
    [](const InputParameters &, InputParameters & rm_params) {
      rm_params.set<unsigned short>("layers") = 1;
    }
);
```

because any coupling-ghosted element is guaranteed to be algebraically ghosted,
and each algebraically ghosted element is guaranteed to be geometrically ghosted
(1 layer coupling ghosting <= 2 layers algebraic ghosting <= 3 layers geometric ghosting).
The following set-up is also valid:

```c++
params.addRelationshipManager("ElementSideNeighborLayers",
                              Moose::RelationshipManagerType::GEOMETRIC |
                                  Moose::RelationshipManagerType::ALGEBRAIC |
                                  Moose::RelationshipManagerType::COUPLING);
```

because there is a (default) 1 layer of side ghosting for all ghosting types
(1 layer coupling ghosting <= 1 layer algebraic ghosting <= 1 layer geometric
ghosting).
The following RM set-up would be **invalid**:

```c++
params.addRelationshipManager(
    "ElementSideNeighborLayers",
    Moose::RelationshipManagerType::GEOMETRIC,
    [](const InputParameters &, InputParameters & rm_params) {
      rm_params.set<unsigned short>("layers") = 1;
    }
);
params.addRelationshipManager(
    "ElementSideNeighborLayers",
    Moose::RelationshipManagerType::ALGEBRAIC,
    [](const InputParameters &, InputParameters & rm_params) {
      rm_params.set<unsigned short>("layers") = 2;
    }
);
```

because there is less geometric ghosting than algebraic. In this scenario we
would likely attempt to access degrees of freedom in the solution vector
corresponding to elements that had been deleted (in a `DistributedMesh`
context), and our simulation would fail.

## Relationship Managers and Actions id=rm_action

As explained above, `RelationshipManagers` are usually added to a simulation
through the `validParams` of `MooseObjects`. However, during simulation setup,
`MooseObjects` themselves do not add the relationship managers because if they
did it would be too late to have any impact on the simulation...remote elements
would already be deleted from the mesh because the mesh is prepared way before
any `MooseObjects` are added (other than `MeshGenerators`). So it is the
`MooseObjectAction` itself that detects whether a `MooseObject` (through its
`validParams`) has signaled that it needs a relationship manager and adds
it. However, if the `MooseObjectAction` is not adding the `MooseObject` , and
the `MooseObject` is being added through a custom `action`, then that custom
action has to be responsible for detecting and adding the associated
relationship managers. The method that the custom `Action` should override to
add relationship managers is
`addRelationshipManagers(Moose::RelationshipManagerType input_rm_type)`. Both
the `ContactAction` in the contact module, and `PorousFlowActionBase` in the
porous flow module provide examples of overriding this method.
