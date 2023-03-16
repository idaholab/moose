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

[UserObjects]
  [to_sub]
    type = LayeredAverage
    direction = x
    num_layers = 10
    variable = to_sub
  []
  [to_sub_elem]
    type = LayeredAverage
    direction = x
    num_layers = 10
    variable = to_sub_elem
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
    positions = '0.2222 0 0 0.61111 0.311111 0.31111 0.76666 0.111111 0.81111'
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
    type = MultiAppGeneralFieldUserObjectTransfer
    to_multi_app = sub
    source_user_object = to_sub
    variable = from_main
    extrapolation_constant = -1
  []

  [to_sub_elem]
    type = MultiAppGeneralFieldUserObjectTransfer
    to_multi_app = sub
    source_user_object = to_sub_elem
    variable = from_main_elem
    extrapolation_constant = -1
  []

  [from_sub]
    type = MultiAppGeneralFieldUserObjectTransfer
    from_multi_app = sub
    source_user_object = to_main
    variable = from_sub
    extrapolation_constant = -1
  []

  [from_sub_elem]
    type = MultiAppGeneralFieldUserObjectTransfer
    from_multi_app = sub
    source_user_object = to_main_elem
    variable = from_sub_elem
    extrapolation_constant = -1
  []
[]
