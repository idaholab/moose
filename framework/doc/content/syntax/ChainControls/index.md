# ChainControls System

The `ChainControls` system is an extension of the [Controls system](syntax/Controls/index.md)
that uses `ChainControl` objects, which instead of working directly with
controllable parameters, work with an additional layer of "control data",
[/ChainControlData.md]. `ChainControl` objects can do the following:

- Declare new control data
- Retrieve control data declared elsewhere
- Change control data values
- Set controllable parameters in MOOSE objects using control data

The main advantage of this additional capability is to chain control operations together,
which is useful for composing complex control systems.

`ChainControlData` is managed by the [/ChainControlDataSystem.md].

## Objects and Associated Actions

!syntax list /ChainControls objects=True actions=False subsystems=False

!syntax list /ChainControls objects=False actions=False subsystems=True

!syntax list /ChainControls objects=False actions=True subsystems=False

