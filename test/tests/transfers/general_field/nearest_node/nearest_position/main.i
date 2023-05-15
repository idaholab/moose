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
  [from_sub]
    initial_condition = -1
  []
  [from_sub_elem]
    order = CONSTANT
    family = MONOMIAL
    initial_condition = -1
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
    overwrite = true
    hide = 'to_sub to_sub_elem'
  []
[]

[Positions]
  [input]
    type = InputPositions
    positions = '1e-6 0 0 0.4 0.6001 0'
  []
[]

[MultiApps]
  [sub]
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
    type = MultiAppGeneralFieldNearestNodeTransfer
    to_multi_app = sub
    source_variable = to_sub
    variable = from_main
  []

  [to_sub_elem]
    type = MultiAppGeneralFieldNearestNodeTransfer
    to_multi_app = sub
    source_variable = to_sub_elem
    variable = from_main_elem
  []

  [from_sub]
    type = MultiAppGeneralFieldNearestNodeTransfer
    from_multi_app = sub
    source_variable = to_main
    variable = from_sub
    use_nearest_position = input
    bbox_factor = 100
  []

  [from_sub_elem]
    type = MultiAppGeneralFieldNearestNodeTransfer
    from_multi_app = sub
    source_variable = to_main_elem
    variable = from_sub_elem
    use_nearest_position = input
    bbox_factor = 100
  []
[]
