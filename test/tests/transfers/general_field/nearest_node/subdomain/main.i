# Base input for testing transfers. It has the following complexities:
# - more than one subapp
# - transfers both from and to the subapps
# - both nodal and elemental variables
# - subapp meshes are not aligned with the main app
# Tests derived from this input may add complexities through command line arguments

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
  []
  [add_block]
    type = ParsedSubdomainMeshGenerator
    input = gmg
    combinatorial_geometry = 'x < 0.6 & y < 0.5'
    block_id = 1
  []
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

[Executioner]
  type = Transient
  num_steps = 1
[]

[Problem]
  solve = false
[]

[Outputs]
  [out]
    type = Exodus
    hide = 'to_sub to_sub_elem'
  []
  execute_on = 'TIMESTEP_END'
[]

[MultiApps]
  [sub]
    # 1 on corner, one in the center and one close to a corner
    # Offsets are added to make sure there are no equidistant nodes / transfer indetermination
    positions = '0.00001 0 0 0.4022222 0.281111 0 0.7232323 0.12323 0'
    type = TransientMultiApp
    app_type = MooseTestApp
    input_files = sub.i
    execute_on = timestep_end
    output_in_position = true
  []
[]

[Transfers]
  [to_sub]
    type = MultiAppGeneralFieldNearestNodeTransfer
    to_multi_app = sub
    source_variable = to_sub
    variable = from_main
    from_blocks = 1
    to_blocks = 1
  []

  [to_sub_elem]
    type = MultiAppGeneralFieldNearestNodeTransfer
    to_multi_app = sub
    source_variable = to_sub_elem
    variable = from_main_elem
    from_blocks = 1
    to_blocks = 1
  []

  [from_sub]
    type = MultiAppGeneralFieldNearestNodeTransfer
    from_multi_app = sub
    source_variable = to_main
    variable = from_sub
    from_blocks = 1
    to_blocks = 1
  []

  [from_sub_elem]
    type = MultiAppGeneralFieldNearestNodeTransfer
    from_multi_app = sub
    source_variable = to_main_elem
    variable = from_sub_elem
    from_blocks = 1
    to_blocks = 1
  []
[]
