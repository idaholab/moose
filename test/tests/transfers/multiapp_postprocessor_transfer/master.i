[Mesh]
  type = GeneratedMesh
  dim = 2
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
    coef = 0.01
  []
  [td]
    type = TimeDerivative
    variable = u
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

[Postprocessors]
  [average]
    type = ElementAverageValue
    variable = u
  []
  [maxValue]
    type = ElementExtremeValue
    variable = u
    value_type = max
  []
  [minValue]
    type = ElementExtremeValue
    variable = u
    value_type = min
  []
[]

[Executioner]
  type = Transient
  num_steps = 5

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = false
  csv = true
[]

[MultiApps]
  [pp_sub]
    app_type = MooseTestApp
    positions = '0.5 0.5 0 0.7 0.7 0'
    execute_on = timestep_end
    type = TransientMultiApp
    input_files = sub.i
  []
[]

[Transfers]
  [pp_transfer]
    type = MultiAppPostprocessorTransfer
    direction = to_multiapp
    multi_app = pp_sub
    from_postprocessor = 'average maxValue minValue'
    to_postprocessor = 'from_master1 from_master2 from_master3'
  []
[]
