# DisplacedProblem

!syntax description /Problem/DisplacedProblem

The `DisplacedProblem` encompasses a normal undisplaced [FEProblemBase.md], which can
be a [FEProblem.md] or an [EigenProblem.md] for example. Function attributes of the
`DisplacedProblem` often forward to the normal encompassed problem, and are only
modified when displacements affect the behavior, such as for [Adaptivity](syntax/Adaptivity/index.md).

The `DisplacedProblem` object contains a displaced nonlinear system, a displaced
auxiliary system and a displaced mesh. The undisplaced mesh can also be obtained from the
`DisplacedProblem`.

!alert note
The `DisplacedProblem` is automatically created by the [CreateDisplacedProblemAction.md]
when the [!param](/Mesh/SetupMeshAction/displacements) parameter is set in the `[Mesh]` block.

!syntax parameters /Problem/DisplacedProblem

!syntax inputs /Problem/DisplacedProblem

!syntax children /Problem/DisplacedProblem
