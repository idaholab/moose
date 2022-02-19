# SONDefinitionFormatter

This class produces a dump of the `InputFileParameters` in the Standard Object
Notation (SON) format for use by the Hierarchical Input Validation Engine
(HIVE) in the NEAMS Workbench. It takes its input from JsonSyntaxTree.

This formatter is used for the `--definition` command line option, described
[here](modules/doc/content/application_usage/command_line_usage.md optional=True). It is also available for the
application developer, either directly or using an [AdvancedOutput](syntax/Outputs/index.md#advanced-output).

## Example output

We reproduce here the output of the `InputParameters` SON dump for the [MooseParsedFunction.md].
We can see that each parameter is listed with additional meta data for how it may be
set in the input file.

```
  'ADParsedFunction_type'{
    InputTmpl=MooseBlock
    InputName="ADParsedFunction"
    InputType=type_sub
    InputDefault="insert_name_here"
    Description="Function created by parsing a string"
    MinOccurs=0
    MaxOccurs=NoLimit
    decl{
      MinOccurs=0
      MaxOccurs=1
      ValType=String
    }
    'active'{
      InputTmpl=MooseParam
      InputType=key_array
      InputName="active"
      Description="If specified only the blocks named will be visited and made active"
      MinOccurs=0
      MaxOccurs=1
      'value'{
        MinOccurs=0
        MaxOccurs=NoLimit
        ValType=String
        InputDefault="__all__"
      }
    } % end parameter active
    'control_tags'{
      InputTmpl=MooseParam
      InputType=key_array
      InputName="control_tags"
      Description="Adds user-defined labels for accessing object parameters via control logic."
      MinOccurs=0
      MaxOccurs=1
      'value'{
        MinOccurs=0
        MaxOccurs=NoLimit
        ValType=String
      }
    } % end parameter control_tags
    'enable'{
      InputTmpl=MooseParam
      InputType=key_value
      InputName="enable"
      Description="Set the enabled status of the MooseObject."
      MinOccurs=0
      MaxOccurs=1
      'value'{
        MinOccurs=1
        MaxOccurs=1
        ValType=String
        ValEnums=[ true false 1 0 on off ]
        InputDefault="1"
      }
    } % end parameter enable
    'execute_on'{
      InputTmpl=MooseParam
      InputType=key_array
      InputName="execute_on"
      Description="The list of flag(s) indicating when this object should be executed, the available options include NONE, INITIAL, LINEAR, NONLINEAR, TIMESTEP_END, TIMESTEP_BEGIN, FINAL, CUSTOM, ALWAYS."
      MinOccurs=0
      MaxOccurs=1
      'value'{
        MinOccurs=0
        MaxOccurs=NoLimit
        ValType=String
        ValEnums=[ "NONE" "INITIAL" "LINEAR" "NONLINEAR" "TIMESTEP_END" "TIMESTEP_BEGIN" "FINAL" "CUSTOM" "ALWAYS" ]
        InputDefault="LINEAR"
      }
    } % end parameter execute_on
    'inactive'{
      InputTmpl=MooseParam
      InputType=key_array
      InputName="inactive"
      Description="If specified blocks matching these identifiers will be skipped."
      MinOccurs=0
      MaxOccurs=1
      'value'{
        MinOccurs=0
        MaxOccurs=NoLimit
        ValType=String
      }
    } % end parameter inactive
    'isObjectAction'{
      InputTmpl=MooseParam
      InputType=key_value
      InputName="isObjectAction"
      Description="Indicates that this is a MooseObjectAction."
      MinOccurs=0
      MaxOccurs=1
      'value'{
        MinOccurs=1
        MaxOccurs=1
        ValType=String
        ValEnums=[ true false 1 0 on off ]
        InputDefault="1"
      }
    } % end parameter isObjectAction
    'type'{
      InputTmpl=MooseParam
      InputType=key_value
      InputName="type"
      MinOccurs=0
      MaxOccurs=1
      'value'{
        MinOccurs=1
        MaxOccurs=1
        ValType=String
        InputDefault="ADParsedFunction"
      }
    } % end parameter type
    'vals'{
      InputTmpl=MooseParam
      InputType=key_array
      InputName="vals"
      Description="Constant numeric values, postprocessor names, or function names for vars."
      MinOccurs=0
      MaxOccurs=1
      'value'{
        MinOccurs=0
        MaxOccurs=NoLimit
        ValType=String
      }
    } % end parameter vals
    ChildAtLeastOne=[ "../../GlobalParams/value/value" "value" ]
    'value'{
      InputTmpl=MooseParam
      InputType=key_array
      InputName="value"
      Description="The user defined function."
      MinOccurs=0
      MaxOccurs=1
      'value'{
        MinOccurs=0
        MaxOccurs=NoLimit
        ValType=String
      }
    } % end parameter value
    'vars'{
      InputTmpl=MooseParam
      InputType=key_array
      InputName="vars"
      Description="Variables (excluding t,x,y,z) that are bound to the values provided by the corresponding items in the vals vector."
      MinOccurs=0
      MaxOccurs=1
      'value'{
        MinOccurs=0
        MaxOccurs=NoLimit
        ValType=String
      }
    } % end parameter vars
  } % end block ADParsedFunction_type
```
