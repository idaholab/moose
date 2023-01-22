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

[Postprocessors]
  [pwr0]
    type = ElementIntegralVariablePostprocessor
    block = 0
    variable = power_density
  []
  [pwr1]
    type = ElementIntegralVariablePostprocessor
    block = 1
    variable = power_density
  []
  [from_sub0]
    type = ElementIntegralVariablePostprocessor
    block = 0
    variable = from_sub
    execute_on = 'transfer'
  []
  [from_sub1]
    type = ElementIntegralVariablePostprocessor
    block = 1
    variable = from_sub
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
    input_files = sub_power_density.i
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
    # N pps are specified on the parent side, where N is the number of subapps
    # 1 pp is specified on the subapp side
    from_postprocessors_to_be_preserved = 'pwr0 pwr1'
    to_postprocessors_to_be_preserved = 'from_parent_pp'
  []

  [from_sub]
    type = MultiAppShapeEvaluationTransfer
    source_variable = sink
    variable = from_sub
    from_multi_app = sub
    execute_on = timestep_end

    # The following inputs specify what postprocessors should be conserved
    # N pps are specified on the parent side, where N is the number of subapps
    # 1 pp is specified on the subapp side
    to_postprocessors_to_be_preserved = 'from_sub0 from_sub1'
    from_postprocessors_to_be_preserved = 'sink'
  []
[]

[Outputs]
  exodus = true
[]
