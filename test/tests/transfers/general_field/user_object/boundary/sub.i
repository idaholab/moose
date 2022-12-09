[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 5
    ny = 5
    nz = 5
    xmax = 0.3
    ymax = 0.3
    zmax = 0.3
  []
  [add_block]
    type = ParsedSubdomainMeshGenerator
    input = gmg
    combinatorial_geometry = 'x > 0.22 & y < 0.23'
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
  [from_main]
    initial_condition = -1
  []
  [from_main_elem]
    order = CONSTANT
    family = MONOMIAL
    initial_condition = -1
  []
  [to_main]
    [InitialCondition]
      type = FunctionIC
      function = '3 + 2*x*x + 3*y*y*y'
    []
  []
  [to_main_elem]
    order = CONSTANT
    family = MONOMIAL
    [InitialCondition]
      type = FunctionIC
      function = '4 + 2*x*x + 3*y*y*y'
    []
  []
[]

[UserObjects]
  [to_main]
    type = LayeredAverage
    direction = x
    num_layers = 10
    variable = to_main
  []
  [to_main_elem]
    type = LayeredAverage
    direction = x
    num_layers = 10
    variable = to_main_elem
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
    hide = 'to_main to_main_elem'
    overwrite = true
  []
[]
