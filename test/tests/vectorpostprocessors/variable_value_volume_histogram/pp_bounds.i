[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 200
  xmin = -5
  xmax = 5
[]

[Variables]
  [c]
    [InitialCondition]
      type = FunctionIC
      function = 'x<2&x>-2'
    []
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = c
  []
  [time]
    type = TimeDerivative
    variable = c
  []
[]

[BCs]
  [all]
    type = DirichletBC
    variable = c
    boundary = 'left right'
    value = 0
  []
[]

[Postprocessors]
  [max]
    type = ParsedPostprocessor
    expression = 't'
    use_t = true
    force_preaux = true
  []
  [min]
    type = ParsedPostprocessor
    expression = 't - 1'
    use_t = true
    force_preaux = true
  []
[]

[VectorPostprocessors]
  [histo]
    type = VariableValueVolumeHistogram
    variable = c
    min_value = min
    max_value = max
    execute_on = 'initial timestep_end'
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 1
  solve_type = PJFNK
[]

[Outputs]
  execute_on = 'initial timestep_end'
  csv = true
[]
