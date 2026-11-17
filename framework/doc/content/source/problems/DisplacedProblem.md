# DisplacedProblem

!syntax description /Problem/DisplacedProblem

The `DisplacedProblem` encompasses a normal undisplaced [FEProblemBase.md], which can
be a [FEProblem.md] or an [EigenProblem.md] for example. Function attributes of the
`DisplacedProblem` often forward to the normal encompassed problem, and are only
modified when displacements affect the behavior, such as for [Adaptivity](syntax/Adaptivity/index.md).

The `DisplacedProblem` object contains a displaced nonlinear system, a displaced
auxiliary system and a displaced mesh. The undisplaced mesh can also be obtained from the
`DisplacedProblem`.

Each node of the displaced mesh is moved by the value of the displacement variables at that
node. A `LAGRANGE` displacement variable can have no degree of freedom on some nodes, for example
first order displacements on QUAD9 elements or second order displacements on the face nodes of
TET14 elements. These nodes are moved by the displacement interpolated from the other nodes of
the element, so the displacement order does not need to match the mesh order. Displacement
variables of other families must have degrees of freedom on every node of the elements they are
defined on.

!alert note
The `DisplacedProblem` is automatically created by the [CreateDisplacedProblemAction.md]
when the [!param](/Mesh/SetupMeshCompleteAction/displacements) parameter is set in the `[Mesh]` block.

!syntax parameters /Problem/DisplacedProblem

!syntax inputs /Problem/DisplacedProblem

!syntax children /Problem/DisplacedProblem
