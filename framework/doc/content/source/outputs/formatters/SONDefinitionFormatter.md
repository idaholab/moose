# SONDefinitionFormatter

This class produces a dump of the `InputFileParameters` in the Standard Object
Notation (SON) format for use by the Hierarchical Input Validation Engine
(HIVE) in the NEAMS Workbench. It takes its input from JsonSyntaxTree.

This formatter is used for the `--definition` command line option, described
[here](application_usage/command_line_usage.md optional=True). It is also available for the
application developer, either directly or using an [AdvancedOutput](syntax/Outputs/index.md#advanced-output).

## Example output

We reproduce here the output of the `InputParameters` SON dump for the [MooseParsedFunction.md].
We can see that each parameter is listed with additional meta data for how it may be
set in the input file.

```
  'ParsedFunction_type'{
    InputTmpl=MooseBlock
    InputName="ParsedFunction"
    InputType=type_sub
    InputDefault="insert_name_here"
    Description="Function created by parsing a string"
    decl{
      MaxOccurs=1
    }
    'active'{
      InputTmpl=MooseParam
      InputType=key_array
      InputName="active"
      Description="If specified only the blocks named will be visited and made active"
      MaxOccurs=1
      'value'{
        InputDefault="__all__"
      }
    }
    'control_tags'{
      InputTmpl=MooseParam
      InputType=key_array
      InputName="control_tags"
      Description="Adds user-defined labels for accessing object parameters via control logic."
      MaxOccurs=1
      'value'{
      }
    }
    'enable'{
      InputTmpl=MooseParam
      InputType=key_value
      InputName="enable"
      Description="Set the enabled status of the MooseObject."
      MaxOccurs=1
      'value'{
        MinOccurs=1
        MaxOccurs=1
        ValEnums=[ true false 1 0 on off ]
        InputDefault="1"
      }
    }
    ChildAtLeastOne=[ "../../GlobalParams/expression/value" "expression" ]
    'expression'{
      InputTmpl=MooseParam
      InputType=key_array
      InputName="expression"
      Description="The user defined function."
      MaxOccurs=1
      'value'{
      }
    }
    'inactive'{
      InputTmpl=MooseParam
      InputType=key_array
      InputName="inactive"
      Description="If specified blocks matching these identifiers will be skipped."
      MaxOccurs=1
      'value'{
      }
    }
    'symbol_names'{
      InputTmpl=MooseParam
      InputType=key_array
      InputName="symbol_names"
      Description="Symbols (excluding t,x,y,z) that are bound to the values provided by the corresponding items in the vals vector."
      MaxOccurs=1
      'value'{
      }
    }
    'symbol_values'{
      InputTmpl=MooseParam
      InputType=key_array
      InputName="symbol_values"
      Description="Constant numeric values, postprocessor names, function names, and scalar variables corresponding to the symbols in symbol_names."
      MaxOccurs=1
      'value'{
      }
    }
    'type'{
      InputTmpl=MooseParam
      InputType=key_value
      InputName="type"
      MaxOccurs=1
      'value'{
        MinOccurs=1
        MaxOccurs=1
        InputDefault="ParsedFunction"
      }
    }
    'vals'{
      InputTmpl=MooseParam
      InputType=key_array
      InputName="vals"
      Description="Constant numeric values, postprocessor names, function names, and scalar variables for vars."
      MaxOccurs=1
      'value'{
      }
    }
    'vars'{
      InputTmpl=MooseParam
      InputType=key_array
      InputName="vars"
      Description="Variables (excluding t,x,y,z) that are bound to the values provided by the corresponding items in the vals vector."
      MaxOccurs=1
      'value'{
      }
    }
  } % end block ParsedFunction_type
```
