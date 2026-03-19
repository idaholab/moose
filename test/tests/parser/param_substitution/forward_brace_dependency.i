late_value = ${target}
target = ${raw 4 1}

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
  [forward_value]
    type = FunctionValuePostprocessor
    function = '${late_value}'
  []
[]

[Outputs]
  csv = true
[]
