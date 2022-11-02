[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -1.0
    xmax = 3.0
    ymin = 0.0
    ymax = 1.0
    nx = 100
    ny = 25
    elem_type = QUAD4
  []
[]

[Problem]
  kernel_coverage_check = false
  skip_nl_system_check = true
  solve = false
  verbose_multiapps = true
[]


[AuxVariables]
  [indicator]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.0
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = 'timestep_end'
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
  [pull_indicator]
    type = MultiAppShapeEvaluationTransfer
    from_multi_app =  solid_domain
    source_variable = solid_indicator
    variable = indicator
    displaced_source_mesh = true
    execute_on = 'initial timestep_begin'
  []
[]
