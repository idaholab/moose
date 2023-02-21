# THMSetupOutputAction

This action creates a parameter [!param](/Outputs/disable_scalars_in_console) in the `[Outputs]` block,
which when set to `TRUE`, sets [!param](/Outputs/Console/execute_scalars_on) to be `NONE`
for all [Console.md] objects, overriding the user-specified value, which has a
default of `INITIAL TIMESTEP_END`. This parameter effectively changes the default
for the [!param](/Outputs/Console/execute_scalars_on) parameter for THM-based
applications. Disabling scalar variables output is often desirable in THM due
to the potentially large number of scalar variables in systems simulations.

!syntax parameters /Outputs/THMSetupOutputAction
