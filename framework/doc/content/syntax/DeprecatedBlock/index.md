# DeprecatedBlock System

The `DeprecatedBlock` system consists of an `Action`, [DeprecatedBlockAction.md], which may be inherited
instead of the regular [Action](source/actions/Action.md), to mark an action syntax as deprecated.

This will add a parameter [!param](/DeprecatedBlock/DeprecatedBlockAction/DEPRECATED) that will be shown in
the syntax's input parameters. It will also print a deprecation message in the console when the deprecated action is used.

!syntax list /DeprecatedBlock objects=True actions=False subsystems=False

!syntax list /DeprecatedBlock objects=False actions=False subsystems=True

!syntax list /DeprecatedBlock objects=False actions=True subsystems=False
