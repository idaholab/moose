# Base input for testing transfers. It has the following complexities:
# - more than one subapp
# - transfers from the subapps
# - both nodal and elemental variables
# - uses the nearest subapp as the source for any given point
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
  [from_sub]
    type = MultiAppGeneralFieldNearestLocationTransfer
    from_multi_app = sub
    source_variable = to_main
    variable = from_sub
    assume_nearest_app_holds_nearest_location = true
    bbox_factor = 100
    # Transfer relies on two nodes that are equidistant to the target point
    search_value_conflicts = false
  []

  [from_sub_elem]
    type = MultiAppGeneralFieldNearestLocationTransfer
    from_multi_app = sub
    source_variable = to_main_elem
    variable = from_sub_elem
    assume_nearest_app_holds_nearest_location = true
    bbox_factor = 100
  []
[]
