[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 3
[]

[AuxVariables]
  [x]
  []
  [y]
    family = MONOMIAL
    order = CONSTANT
  []
  [x_apps]
    family = MONOMIAL
    order = CONSTANT
  []
  [y_apps]
  []
[]

[ICs]
  [x]
    type = FunctionIC
    function = x
    variable = x
  []
  [y]
    type = FunctionIC
    function = y
    variable = y
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [sub]
    type = QuadraturePointMultiApp
    input_files = 'sub_app.i'
    run_in_position = true
    cli_args = 'Postprocessors/average_x/type=ElementAverageValue;Postprocessors/average_x/variable=x;Postprocessors/average_y/type=ElementAverageValue;Postprocessors/average_y/variable=y'
  []
[]

[Transfers]
  # Check that sending data to the child app works
  [sending_x]
    type = MultiAppVariableValueSamplePostprocessorTransfer
    source_variable = x
    to_multi_app = sub
    postprocessor = incoming_x
  []
  [sending_y]
    type = MultiAppVariableValueSamplePostprocessorTransfer
    source_variable = y
    to_multi_app = sub
    postprocessor = incoming_y
  []

  # And receiving from the child apps
  [receiving_x]
    type = MultiAppPostprocessorInterpolationTransfer
    postprocessor = average_x
    from_multi_app = sub
    variable = x_apps
    num_points = 4
  []
  [receving_y]
    type = MultiAppPostprocessorInterpolationTransfer
    postprocessor = average_y
    from_multi_app = sub
    variable = y_apps
    num_points = 4
  []
[]
