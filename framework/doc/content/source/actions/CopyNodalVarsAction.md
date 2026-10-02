# CopyNodalVarsAction

!syntax description /Variables/CopyNodalVarsAction

This action handles the `initial_from_file_var` [`Variables`](syntax/Variables/index.md) parameter
as used below,

!listing restart/scalar-var/part2.i block=Variables

This syntax supports restarting variables from either an `Exodus` file or a MOOSE `Checkpoint`
file. When the [Mesh](syntax/Mesh/index.md) is read from a checkpoint file, the requested
variables are copied from that checkpoint's stored solution; unlike the `Exodus` path, this
supports distributed meshes and high-order elemental variables. See
[MooseVariableBase.md#restart] for the details and constraints of each source. More information
about alternative options to restart simulations may be found in the
[Restart/Recover system](/application_usage/restart_recover.md optional=True).

!syntax parameters /Variables/CopyNodalVarsAction
