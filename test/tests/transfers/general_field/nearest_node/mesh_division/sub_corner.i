# Sub-app input for use with main_match_subapps_corner_base.i.
# The mesh is a small square in the lower-left, designed to be placed at a corner of the
# parent domain via 'positions' so that each instance falls within exactly one quadrant of
# the parent mesh division.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3
  xmin = 0
  ymin = 0
  xmax = 0.1
  ymax = 0.1
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
      function = '1 + 20*x + 300*y*y*y'
    []
  []
  [to_main_elem]
    order = CONSTANT
    family = MONOMIAL
    [InitialCondition]
      type = FunctionIC
      function = '2 + 20*x + 300*y*y*y'
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
  exodus = true
[]
