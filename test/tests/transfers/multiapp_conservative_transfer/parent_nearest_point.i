[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 1
    ymax = 1
    nx = 10
    ny = 10
  []
  [block1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    block_id = 1
    bottom_left = '0.5 0 0'
    top_right = '1 1 0'
  []
[]

[Variables]
  [power_density]
  []
[]

[Functions]
  [pwr_func]
    type = ParsedFunction
    expression = '1e3*x*(1-x)+5e2'
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = power_density
  []

  [coupledforce]
    type = BodyForce
    variable = power_density
    function = pwr_func
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = power_density
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = power_density
    boundary = right
    value = 1e3
  []
[]

[AuxVariables]
  [from_sub]
  []
[]

[VectorPostprocessors]
  [from_nearest_point]
    type = NearestPointIntegralVariablePostprocessor
    variable = power_density
    points = '0 0.5 0 1 0.5 0'
  []

  [to_nearest_point]
    type = NearestPointIntegralVariablePostprocessor
    variable = from_sub
    points = '0 0.5 0 1 0.5 0'
    execute_on = 'transfer'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    input_files = sub_nearest_point.i
    positions = '0 0 0 0.5 0 0'
    execute_on = timestep_end
  []
[]

[Transfers]
  [to_sub]
    type = MultiAppShapeEvaluationTransfer
    source_variable = power_density
    variable = from_parent
    to_multi_app = sub
    execute_on = timestep_end

    # The following inputs specify what postprocessors should be conserved
    # 1 NearestPointIntegralVariablePostprocessor is specified on the parent
    # side with N points, where N is the number of subapps
    # 1 pp is specified on the subapp side
    from_postprocessors_to_be_preserved = 'from_nearest_point'
    to_postprocessors_to_be_preserved = 'from_parent_pp'
  []

  [from_sub]
    type = MultiAppShapeEvaluationTransfer
    source_variable = sink
    variable = from_sub
    from_multi_app = sub
    execute_on = timestep_end

    # The following inputs specify what postprocessors should be conserved
    # 1 NearestPointIntegralVariablePostprocessor is specified on the parent
    # with N points, where N is the number of subapps
    # 1 pp is specified on the subapp side
    to_postprocessors_to_be_preserved = 'to_nearest_point'
    from_postprocessors_to_be_preserved = 'sink'
  []
[]

[Outputs]
  csv = true
  exodus = true
[]
