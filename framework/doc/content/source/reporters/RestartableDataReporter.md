# RestartableDataReporter

A Reporter object for outputting [Restartable.md] data in a human-readable form. This includes
restartable metadata, such as mesh metadata. This object is particularly useful for inspecting
restartable data for the purposes of debugging.

Only data types that have a `to_json` specialization support having their values output. If the
[!param](/Reporters/RestartableDataReporter/allow_unimplemented) parameter is set to true,
restartable data with unimplemented specializations will still have their auxiliary information output
(type, whether or not the data is loaded/stored, etc).

The [!param](/Reporters/RestartableDataReporter/include) and [!param](/Reporters/RestartableDataReporter/exclude)
parameters enable the filtering of data by name. With these parameters, you can use `*` as a
wildcard match of any number of characters and `?` as a wildcard match for a single character.

## Example Input Syntax

The following input file snippet demonstrates the use of the `RestartableDataReporter` object.

!listing restartable_data_reporter/restartable_data_reporter.i block=Reporters/data

!syntax parameters /Reporters/RestartableDataReporter

!syntax inputs /Reporters/RestartableDataReporter

!syntax children /Reporters/RestartableDataReporter
