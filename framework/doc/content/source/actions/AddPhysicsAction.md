# AddPhysicsAction

!syntax description /Physics/AddPhysicsAction

Additional documentation may be found on the [syntax page](syntax/Physics/index.md) and
in each `Physics` object's individual documentation.

The `AddPhysicsAction` encompasses most of the other `AddXXXAction` in order to offer maximum flexibility
in implementing your `Physics`, with the notable exceptions of:

- mesh generation is not included, because this is more a property of the system you are modeling than the physics you are modeling
- stochastic analysis-related quantities, which should be handled upstream of the `Physics` definition

!syntax parameters /Physics/AddPhysicsAction
