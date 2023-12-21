base_value = 3

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
[]

[MeshDivisions]
  [middle_sub]
    type = CartesianGridDivision
    # excludes the nodes on the left boundary
    bottom_left = '0.0008 0.20001 0'
    top_right = '0.6001 1 0'
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
      function = '${base_value} + 2*x*x + 3*y*y*y'
    []
  []
  [to_main_elem]
    order = CONSTANT
    family = MONOMIAL
    [InitialCondition]
      type = FunctionIC
      function = '${base_value} + 1 + 2*x*x + 3*y*y*y'
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
    hide = 'to_main to_main_elem div'
    overwrite = true
  []
[]

# For debugging purposes
[AuxVariables]
  [div]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [mesh_div]
    type = MeshDivisionAux
    variable = div
    mesh_division = 'middle_sub'
  []
[]
