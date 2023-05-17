# Base input for testing transfers. It has the following complexities:
# - more than one subapp
# - transfers both from and to the subapps
# - both nodal and elemental variables
# Tests derived from this input may add complexities through command line arguments

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[AuxVariables]
  [from_sub]
    initial_condition = -1
  []
  [from_sub_elem]
    order = CONSTANT
    family = MONOMIAL
    initial_condition = -1
  []
  [to_sub]
    [InitialCondition]
      type = FunctionIC
      function = '1 + 2*x*x + 3*y*y*y'
    []
  []
  [to_sub_elem]
    order = CONSTANT
    family = MONOMIAL
    [InitialCondition]
      type = FunctionIC
      function = '2 + 2*x*x + 3*y*y*y'
    []
  []
[]

[UserObjects]
  [to_sub]
    type = LayeredAverage
    direction = x
    num_layers = 10
    variable = to_sub
    execute_on = TIMESTEP_BEGIN
  []
  [to_sub_elem]
    type = LayeredAverage
    direction = x
    num_layers = 10
    variable = to_sub_elem
    execute_on = TIMESTEP_BEGIN
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
  verbose_multiapps = true
[]

[Outputs]
  [out]
    type = Exodus
    hide = 'to_sub to_sub_elem'
    overwrite = true
  []
[]

[Positions]
  [input]
    type = InputPositions
    positions = '1e-6 0 0 0.4 0.4001 0 0.700001 0.1 0'
  []
[]

[MultiApps]
  [sub]
    # 1 on corner, one in the center and one close to a corner
    type = FullSolveMultiApp
    positions_objects = input
    app_type = MooseTestApp
    input_files = sub.i
    output_in_position = true
    execute_on = TIMESTEP_END
  []
[]

[Transfers]
  [to_sub]
    type = MultiAppGeneralFieldUserObjectTransfer
    to_multi_app = sub
    source_user_object = to_sub
    variable = from_main
  []

  [to_sub_elem]
    type = MultiAppGeneralFieldUserObjectTransfer
    to_multi_app = sub
    source_user_object = to_sub_elem
    variable = from_main_elem
  []

  [from_sub]
    type = MultiAppGeneralFieldUserObjectTransfer
    from_multi_app = sub
    source_user_object = to_main
    variable = from_sub
    use_nearest_app = true
    bbox_factor = 100
  []

  [from_sub_elem]
    type = MultiAppGeneralFieldUserObjectTransfer
    from_multi_app = sub
    source_user_object = to_main_elem
    variable = from_sub_elem
    use_nearest_app = true
    bbox_factor = 100
  []
[]
