# ChainControlSetupAction

This [Action.md] is responsible for setup and checks that should occur after
all [ChainControls](syntax/ChainControls/index.md) have been added. It does
the following:

- Checks that all requested [/ChainControlData.md] has been declared and throws
  an error if not.
- Calls `init()` on all `ChainControls`.
- Adds dependencies for each `ChainControl` on other `ChainControls` based on
  the dependencies of the `ChainControlData`; if the `ChainControl` depends on
  a `ChainControlData`, then it depends on the `ChainControl` that declared that data.
