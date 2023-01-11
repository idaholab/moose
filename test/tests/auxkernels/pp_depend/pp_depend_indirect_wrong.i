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
  # This FunctionValuePostprocessor uses an outdated value for ptime
  [t_pp1]
    type = FunctionValuePostprocessor
    function = t_func
  []

  [ptime_pp]
    type = TimePostprocessor
  []

  # This FunctionValuePostprocessor uses the current value for ptime
  # This is construction order dependent
  [t_pp2]
    type = FunctionValuePostprocessor
    function = t_func
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
