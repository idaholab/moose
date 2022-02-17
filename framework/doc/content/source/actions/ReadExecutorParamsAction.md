# ReadExecutorParamsAction

!syntax description /Executors/ReadExecutorParamsAction

This [MooseObjectAction](MooseObjectAction.md) checks whether a [Preconditioner](syntax/Preconditioning/index.md) has
been specified in the `[Executor]` block, and if not creates a default preconditioner.

The addition of the default preconditioner is similar to what is performed by the [CreateExecutionerAction.md].

!syntax parameters /Executors/ReadExecutorParamsAction
