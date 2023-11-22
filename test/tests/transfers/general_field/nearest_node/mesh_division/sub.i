[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmax = 0.8
  ymax = 0.8
[]

[MeshDivisions]
  [middle_sub]
    type = CartesianGridDivision
    bottom_left = '0.21 0.21 0'
    top_right = '0.81 0.81 0'
    nx = 4
    ny = 4
    nz = 1
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
