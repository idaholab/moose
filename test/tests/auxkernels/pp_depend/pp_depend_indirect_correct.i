[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [u]
  []
[]

[Functions]
  [t_func]
    type = ParsedFunction
    expression = ptime
    symbol_names = ptime
    symbol_values = ptime_pp
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[Postprocessors]
  [t_pp1]
    type = FunctionValuePostprocessor
    function = t_func
    indirect_dependencies = ptime_pp
  []

  [ptime_pp]
    type = TimePostprocessor
  []

  [t_pp2]
    type = FunctionValuePostprocessor
    function = t_func
    indirect_dependencies = ptime_pp
  []
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 5
[]

[Outputs]
  csv = true
[]
