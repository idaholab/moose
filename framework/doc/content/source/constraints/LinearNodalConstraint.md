# LinearNodalConstraint

!syntax description /Constraints/LinearNodalConstraint

The LinearNodalConstraint class constrains the secondary node to move as a linear combination of primary nodes. Primary node ids and corresponding weights are taken as input along with the secondary node set or secondary node ids, which are mutually exclusive. This nodal constraint can be enforced either using the penalty formulation or the kinematic formulation.

Each weight is applied to the primary node at the same position in the `primary` list, so the constrained variable must be defined on every primary node. The elements the primary nodes need are gathered and retained on the processes that own a secondary node, which lets the constraint be used on a distributed mesh. The node lists and that ghosting are rebuilt on every mesh change, so the constraint also remains valid under mesh adaptivity.

!syntax parameters /Constraints/LinearNodalConstraint

!syntax inputs /Constraints/LinearNodalConstraint

!syntax children /Constraints/LinearNodalConstraint

!bibtex bibliography
