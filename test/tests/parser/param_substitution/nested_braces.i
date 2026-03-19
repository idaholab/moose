# Base values used by the nested brace-expression substitutions below.
foo1 = 41
foo2 = 42
num = 1
x = 2
prefix_foo1 = 7

[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [u]
  []
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  # Nested expressions can synthesize the command name before the outer command runs.
  [implicit_replace_nested_cmd]
    type = FunctionValuePostprocessor
    function = '${${raw re place} ${raw foo ${num}}}'
  []
  # Nested replace results can feed directly into fparse arguments.
  [nested_fparse]
    type = FunctionValuePostprocessor
    function = '${fparse ${replace ${raw foo ${num}}} + ${foo2}}'
  []
  # Nested expressions inside a single argument token should be preserved.
  [nested_token_replace]
    type = FunctionValuePostprocessor
    function = '${replace prefix_${raw foo ${num}}}'
  []
  # A previously impossible chain now works: fparse(unit(fparse(unit(...)))).
  [nested_units]
    type = FunctionValuePostprocessor
    function = '${fparse ${units ${fparse ${units ${x} m -> cm} / 2} cm -> m} + 1}'
  []
[]

[Outputs]
  csv = true
[]
