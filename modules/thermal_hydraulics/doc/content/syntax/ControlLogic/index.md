# ControlLogic System

The `ControlLogic` system is an extension of MOOSE's [Controls system](syntax/Controls/index.md).
Standard MOOSE `Control`s can be created within a `[ControlLogic]` block or a
`[Controls]` block. `THMControl`s, however, which have additional functionality,
can only be created within a `[ControlLogic]` block.

While MOOSE `Control`s work "directly" with controllable parameters,
`THMControl`s can work with an additional layer, +control data+. A `THMControl` can:

- Declare new control data
- Retrieve control data declared elsewhere
- Change control data values
- Set controllable parameters in MOOSE objects using control data

The main advantage of this additional capability is to chain control operations together,
which is useful for composing realistic control systems.

## Objects and Associated Actions

!syntax list /ControlLogic objects=True actions=False subsystems=False

!syntax list /ControlLogic objects=False actions=False subsystems=True

!syntax list /ControlLogic objects=False actions=True subsystems=False
