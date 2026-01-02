# JsonInputFileFormatter

This class produces a dump of the `InputParameters` that appears like the normal input
file syntax.
It is different from the [InputFileFormatter.md] in that it takes its input from JsonSyntaxTree.

This formatter is used for the `--dump` command line option, described
[here](application_usage/command_line_usage.md optional=True). It is also available for the
application developer, either directly or using an [AdvancedOutput](syntax/Outputs/index.md#advanced-output).

## Example output

We reproduce here the output of the `InputParameters` JSON dump for the [Functions](syntax/Functions/index.md)
block and the [MooseParsedFunction.md]. We can see the characteristic HIT syntax format, each parameter,
their default value if any, whether they are required or not, and each parameter's description.
This is output for every hierarchical level in the syntax.

```
[Functions]

  [./*]
    active       = '__all__ '   # "If specified only the blocks named will be visited and made active"
                                # Unit: ""
                                # Group: ""
    control_tags = (no_default) # "Adds user-defined labels for accessing object parameters via control
                                # logic."
                                # Unit: ""
                                # Group: "Advanced"
    inactive     = (no_default) # "If specified blocks matching these identifiers will be skipped."
                                # Unit: ""
                                # Group: ""
    type         = (required)   # "A string representing the Moose Object that will be built by this Action"
                                # Unit: ""
                                # Group: ""

    [./<types>]

      [./<ParsedFunction>]
        # "Function created by parsing a string"
        control_tags  = (no_default)   # "Adds user-defined labels for accessing object parameters via control
                                       # logic."
                                       # Unit: ""
                                       # Group: "Advanced"
        enable        = 1              # "Set the enabled status of the MooseObject."
                                       # Unit: ""
                                       # Group: "Advanced"
        expression    = (required)     # "The user defined function."
                                       # Unit: ""
                                       # Group: ""
        symbol_names  = (required)     # "Symbols (excluding t,x,y,z) that are bound to the values provided
                                       # by the corresponding items in the symbol_values vector."
                                       # Unit: ""
                                       # Group: ""
        symbol_values = (required)     # "Constant numeric values, postprocessor names, function names,
                                       # and scalar variables corresponding to the symbols in symbol_names."
                                       # Unit: ""
                                       # Group: ""
        type          = ParsedFunction # ""
                                       # Unit: ""
                                       # Group: ""
      [../]
```
