# YAMLFormatter

This class produces a yaml dump of the `InputParameters` that is machine parsable by
any YAML formatter.

This formatter is used for the `--yaml` command line option, described
[here](application_usage/command_line_usage.md optional=True). It is also available for the
application developer, either directly or using an [AdvancedOutput](syntax/Outputs/index.md#advanced-output).

## Example output

We reproduce here the output of the `InputParameters` YAML dump for the [Functions](syntax/Functions/index.md)
block and the [MooseParsedFunction.md]. We can see metadata about each parameter, such as their
C++ type, their default, or their description.

```
- name: /Functions/ParsedFunction
  description: |

  parameters:
  - name: control_tags
    required: No
    default: !!str
    cpp_type: std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >
    group_name: 'Advanced'
    doc_unit:
    doc_range:
    description: |
      Adds user-defined labels for accessing object parameters via control logic.
  - name: enable
    required: No
    default: !!str 1
    cpp_type: bool
    group_name: 'Advanced'
    doc_unit:
    doc_range:
    description: |
      Set the enabled status of the MooseObject.
  - name: expression
    required: Yes
    default: !!str
    cpp_type: FunctionExpression
    group_name:
    doc_unit:
    doc_range:
    description: |
      The user defined function.
  - name: symbol_names
    required: Yes
    default: !!str
    cpp_type: std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >
    group_name:
    doc_unit:
    doc_range:
    description: |
      Symbols (excluding t,x,y,z) that are bound to the values provided by the corresponding items in the symbol_values vector.
  - name: symbol_values
    required: Yes
    default: !!str
    cpp_type: std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >
    group_name:
    doc_unit:
    doc_range:
    description: |
      Constant numeric values, postprocessor names, function names, and scalar variables corresponding to the symbols in symbol_names.
  - name: type
    required: No
    default: !!str ParsedFunction
    cpp_type: std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >
    group_name:
    doc_unit:
    doc_range:
    description: |

  subblocks:
```
