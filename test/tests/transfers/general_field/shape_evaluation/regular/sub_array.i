[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmax = 0.2
  ymax = 0.2
[]

[AuxVariables]
  [from_main]
    initial_condition = '-1 -1'
    components = 2
  []
  [from_main_elem]
    order = CONSTANT
    family = MONOMIAL
    initial_condition = '-1 -1'
    components = 2
  []
  [to_main]
    [InitialCondition]
      type = ArrayFunctionIC
      function = '3+2*x*x+3*y*y*y 5+2*x*x+3*y*y*y'
    []
    components = 2
  []
  [to_main_elem]
    order = CONSTANT
    family = MONOMIAL
    [InitialCondition]
      type = ArrayFunctionIC
      function = '4+2*x*x+3*y*y*y 6+2*x*x+3*y*y*y'
    []
    components = 2
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
