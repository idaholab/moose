# Base input for testing transfers. It has the following complexities:
# - more than one subapp
# - transfers both from and to the subapps
# - both nodal and elemental variables
# - subapp meshes are not aligned with the main app
# Tests derived from this input may add complexities through command line arguments

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 5
    ny = 5
    nz = 5
  []
  [add_block]
    type = ParsedSubdomainMeshGenerator
    input = gmg
    combinatorial_geometry = 'x < 0.5 & y < 0.5'
    block_id = 1
  []
  [add_internal_sideset]
    type = SideSetsBetweenSubdomainsGenerator
    input = add_block
    primary_block = 0
    paired_block = 1
    new_boundary = internal
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
  verbose_multiapps = true
[]

[Outputs]
  [out]
    type = Exodus
    hide = 'to_sub to_sub_elem'
    overwrite = true
  []
[]

[MultiApps]
  [sub]
    # 1 on corner, one in the center and one close to a corner
    # The subapp mesh is a 0.3-sized cube, no overlap
    positions = '0.2222 0.00002 0.0001 0.61111 0.311111 0.31211 0.76666 0.111114 0.81111'
    type = TransientMultiApp
    app_type = MooseTestApp
    input_files = sub.i
    execute_on = timestep_end
    # Facilitates debugging
    output_in_position = true
  []
[]

[Transfers]
  # Boundary restrictions are added in the tests specification
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
  []

  [from_sub_elem]
    type = MultiAppGeneralFieldNearestNodeTransfer
    from_multi_app = sub
    source_variable = to_main_elem
    variable = from_sub_elem
  []
[]
