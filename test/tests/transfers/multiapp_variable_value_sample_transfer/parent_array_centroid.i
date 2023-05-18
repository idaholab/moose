[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[Variables]
  [u_parent]
    components = 2
  []
[]

[AuxVariables]
  [u_sub]
    family = MONOMIAL
    order = CONSTANT
    components = 2
  []
[]

[Functions]
  [u0_fun]
    type = ParsedFunction
    expression = 'x'
  []
  [u1_fun]
    type = ParsedFunction
    expression = 'y'
  []
[]

[ICs]
  [uic]
    type = ArrayFunctionIC
    variable = u_parent
    function = 'u0_fun u1_fun'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]

[Positions]
  [centroid]
    type = ElementCentroidPositions
  []
[]

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    input_files = sub_array_sample.i
    execute_on = timestep_end
    positions_objects = 'centroid centroid'
  []
[]

[Transfers]
  [to_transfer]
    type = MultiAppVariableValueSamplePostprocessorTransfer
    to_multi_app = sub
    postprocessor = from_parent
    source_variable = u_parent
    map_array_variable_components_to_child_apps = true
  []
  [from_transfer]
    type = MultiAppVariableValueSamplePostprocessorTransfer
    from_multi_app = sub
    postprocessor = to_parent
    source_variable = u_sub
    map_array_variable_components_to_child_apps = true
  []
[]
