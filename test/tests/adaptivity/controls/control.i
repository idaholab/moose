[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  ny = 10
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Transient
  num_steps = 3
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  nl_abs_tol = 1e-8
[]

[Functions]
  [turn_on_adapt]
    type = ParsedFunction
    expression = 't > 1.0'
  []
[]

[Controls]
  [control_adapt]
    type = BoolFunctionControl
    function = 'turn_on_adapt'
    parameter = 'adapt_tag/*/enable'
    execute_on = 'TIMESTEP_BEGIN'
  []
[]

[Adaptivity]
  marker = marker
  control_tags = 'adapt_tag'

  [Markers/marker]
    type = UniformMarker
    mark = 'REFINE'
  []
[]

[Postprocessors]
  [elem]
    type = NumElements
  []
[]

[Outputs]
  csv = true
  execute_on = 'TIMESTEP_END'
[]
