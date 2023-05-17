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
  [single]
    type = InputPositions
    # Tiny offset to steer clear of equidistance
    positions = '0.000001 0.0000000001 0'
  []
  [partition_app_domain]
    type = InputPositions
    # Tiny offsets to steer clear of equi-distance
    # The top left and bottom right are closer to top right than to the bottom left corner
    # This makes it so that the bottom left "nearest-area" is larger
    # and the top-right "nearest" area is smaller
    positions = '0.1 0.12 0
                 0.900008 0.130000000001 0
                 0.85000007 0.8500001 0
                 0.10001 0.75003 0'
  []
[]

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    positions_objects = single
    app_type = MooseTestApp
    input_files = sub_holes.i
    output_in_position = true
    execute_on = TIMESTEP_END
  []
[]

[Transfers]
  [from_sub]
    type = MultiAppGeneralFieldNearestNodeTransfer
    from_multi_app = sub
    source_variable = to_main
    variable = from_sub
    use_nearest_position = partition_app_domain
    search_value_conflicts = true
    bbox_factor = 10
  []

  [from_sub_elem]
    type = MultiAppGeneralFieldNearestNodeTransfer
    from_multi_app = sub
    source_variable = to_main_elem
    variable = from_sub_elem
    use_nearest_position = partition_app_domain
    search_value_conflicts = true
    bbox_factor = 10
  []
[]
