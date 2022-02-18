# JsonInputFileFormatter

This class produces produces a dump of the `InputParameters` that appears like the normal input
file syntax.
It is different from the [InputFileFormatter.md] in that it takes its input from JsonSyntaxTree.

This formatter is used for the `--dump` command line option, described
[here](modules/doc/content/application_usage/command_line_usage.md optional=True). It is also available for the
application developer, either directly or using an [AdvancedOutput](syntax/Outputs/index.md#advanced-output).

## Example output

We reproduce here the output of the `InputParameters` JSON dump for the [Functions](syntax/Functions/index.md)
block and the [MooseParsedFunction.md]. We can see the characteristic HIT syntax format, each parameter,
their default value if any, whether they are required or not, and each parameter's description.
This is output for every hierarchical level in the syntax.

```
[Functions]

  [./*]
    active         = '__all__ '   # "If specified only the blocks named will be visited and made active"
                                  # Group: ""
    inactive       = (no_default) # "If specified blocks matching these identifiers will be skipped."
                                  # Group: ""
    isObjectAction = 1            # "Indicates that this is a MooseObjectAction."
                                  # Group: ""
    type           = (required)   # "A string representing the Moose Object that will be built by this Action"
                                  # Group: ""

    [./<types>]

      [./<ADParsedFunction>]
        # "Function created by parsing a string"
        control_tags = (no_default)     # "Adds user-defined labels for accessing object parameters via
                                        # control logic."
                                        # Group: "Advanced"
        enable       = 1                # "Set the enabled status of the MooseObject."
                                        # Group: "Advanced"
        execute_on   = LINEAR           # "The list of flag(s) indicating when this object should be executed,
                                        # the available options include NONE, INITIAL, LINEAR, NONLINEAR,
                                        # TIMESTEP_END, TIMESTEP_BEGIN, FINAL, CUSTOM, ALWAYS."
                                        # Group: ""
        type         = ADParsedFunction # ""
                                        # Group: ""
        vals         = (no_default)     # "Constant numeric values, postprocessor names, or function names
                                        # for vars."
                                        # Group: ""
        value        = (required)       # "The user defined function."
                                        # Group: ""
        vars         = (no_default)     # "Variables (excluding t,x,y,z) that are bound to the values provided
                                        # by the corresponding items in the vals vector."
                                        # Group: ""
      [../]
```
