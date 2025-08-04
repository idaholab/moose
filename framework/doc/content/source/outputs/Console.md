# Console

The Console object in MOOSE is a buffered ostringstream object that should be used to for
all screen based output. Each MooseObject derived class has access to the _console
member for screen writing. Output to this object can be controlled in various ways from
the `[Outputs]` block. Information can be disabled, output at a lower frequency, colored,
or color removed.

This output supports an additional execution schedule, with the `execute_on` flag `MULTIAPP_FIXED_POINT_ITERATION_END`, corresponding
to the end of each successful MultiApp fixed point iteration (if there is iteration).
In this case it is recommended to set [!param](/Outputs/Console/new_row_detection_columns) to `all`.

## Multiapp Output

When using the MOOSE Multiapp system, Output to the _console object will be automatically
indented and labeled according to the multiapp level. This can make it easier to identify
where output is coming from in a more complex simulation.

!syntax parameters /Outputs/Console

!syntax inputs /Outputs/Console

!syntax children /Outputs/Console

!bibtex bibliography
