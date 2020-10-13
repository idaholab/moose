# JSON

The `JSONOutput` object exists to output values from the [Reporters/index.md] in the [!ac](JSON)
format.  Reporter values are arbitrary types, as such the JSON output supports the output of any
type if the correct output helper function exists.

## Example

The following input snippet demonstrates the default method for enabling JSON output.

!listing test/tests/outputs/json/basic/json.i block=Outputs

The resulting [!ac](JSON) output is generally in the following form, where the time information are
provided at the top level and reporters values are nested within a list for the time steps.

!listing reporters/iteration_info/gold/limit_out.json

## Distributed Output

If a Reporter value is computed with `REPORTER_MODE_DISTRIBUTED` (see [Reporters/index.md) a JSON
file for each process will automatically be created with the distributed data and the total number
of parts and part number for the file will be included in the output.

## `to_json` function

In similar fashion to the [DataIO.md] functions used for
[restart and recovery](restart_recover.md optional=True) a `to_json` method must exist for a
type to be supported for output to a [!ac](JSON) file. The function is defined in the
[nlohmann/json](https://github.com/nlohmann/json) library---the library relied upon by MOOSE for
JSON support---for this purpose.

For example, the following code snippets show the declaration and definition of the `to_json` method
for the `MooseApp` object.

!listing framework/include/outputs/JsonIO.h re=^\s*(?P<content>[^\n]*?)\s*// MooseDocs:to_json

!listing framework/src/outputs/JsonIO.C start=MooseDocs:to_json_start end=MooseDocs:to_json_end include-start=False

!syntax parameters /Outputs/JSON

!syntax inputs /Outputs/JSON

!syntax children /Outputs/JSON
