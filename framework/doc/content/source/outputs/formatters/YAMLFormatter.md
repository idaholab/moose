# YAMLFormatter

This class produces produces a yaml dump of the `InputParameters` that is machine parsable by
any YAML formatter.

This formatter is used for the `--yaml` command line option, described
[here](modules/doc/content/application_usage/command_line_usage.md optional=True). It is also available for the
application developer, either directly or using an [AdvancedOutput](syntax/Outputs/index.md#advanced-output).

## Example output

We reproduce here the output of the `InputParameters` YAML dump for the [Functions](syntax/Functions/index.md)
block and the [MooseParsedFunction.md]. We can see metadata about each parameter, such as their
C++ type, their default, or their description.

```
- name: /Functions/ADParsedFunction
  description: |

  parameters:
  - name: control_tags
    required: No
    default: !!str
    cpp_type: std::__1::vector<std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> >, std::__1::allocator<std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> > > >
    group_name: 'Advanced'
    description: |
      Adds user-defined labels for accessing object parameters via control logic.
  - name: enable
    required: No
    default: !!str 1
    cpp_type: bool
    group_name: 'Advanced'
    description: |
      Set the enabled status of the MooseObject.
  - name: execute_on
    required: No
    default: !!str LINEAR
    cpp_type: ExecFlagEnum
    group_name:
    description: |
      The list of flag(s) indicating when this object should be executed, the available options include NONE, INITIAL, LINEAR, NONLINEAR, TIMESTEP_END, TIMESTEP_BEGIN, FINAL, CUSTOM, ALWAYS.
  - name: type
    required: No
    default: !!str ADParsedFunction
    cpp_type: std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> >
    group_name:
    description: |

  - name: vals
    required: No
    default: !!str
    cpp_type: std::__1::vector<std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> >, std::__1::allocator<std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> > > >
    group_name:
    description: |
      Constant numeric values, postprocessor names, or function names for vars.
  - name: value
    required: Yes
    default: !!str
    cpp_type: FunctionExpression
    group_name:
    description: |
      The user defined function.
  - name: vars
    required: No
    default: !!str
    cpp_type: std::__1::vector<std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> >, std::__1::allocator<std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> > > >
    group_name:
    description: |
      Variables (excluding t,x,y,z) that are bound to the values provided by the corresponding items in the vals vector.
  subblocks:

```
