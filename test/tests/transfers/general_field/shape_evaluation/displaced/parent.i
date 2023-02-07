[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -0.5
    xmax = 1.5
    ymin = 0.0
    ymax = 0.5
    nx = 50
    ny = 3
    elem_type = QUAD9
  []
[]

[Problem]
  kernel_coverage_check = false
  skip_nl_system_check = true
  solve = false
  verbose_multiapps = true
[]

[AuxVariables]
  [indicator_const_mon]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 0.0
  []
  [indicator_nodal]
    initial_condition = 0.0
  []
  [indicator_higher_order]
    family = MONOMIAL
    order = THIRD
    initial_condition = 0.0
  []
[]

[Executioner]
  type = Transient
  dt = 0.05
  num_steps = 10
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[MultiApps]
  [solid_domain]
    type = TransientMultiApp
    input_files = child.i
    execute_on = 'initial timestep_begin'
  []
[]

[Transfers]
  [pull_indicator_constmon]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    from_multi_app = solid_domain
    source_variable = solid_indicator
    variable = indicator_const_mon
    displaced_source_mesh = true
    execute_on = 'initial timestep_begin'
  []
  [pull_indicator_nodal]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    from_multi_app = solid_domain
    source_variable = solid_indicator
    variable = indicator_nodal
    displaced_source_mesh = true
    execute_on = 'initial timestep_begin'
  []
  [pull_indicator_higher]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    from_multi_app = solid_domain
    source_variable = solid_indicator
    variable = indicator_higher_order
    displaced_source_mesh = true
    execute_on = 'initial timestep_begin'
  []
[]
