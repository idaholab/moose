[Mesh]
  second_order = true
  [MainMesh]
    type = GeneratedMeshGenerator
    dim = 1
    ymax = 1.0
    ny = 10
  []
[]

[AuxVariables]
  [Variable_1]
    order = SECOND
    family = LAGRANGE
  []
  [Variable_2]
    order = CONSTANT
    family = MONOMIAL
  []
  [Variable_3]
    order = SECOND
    family = LAGRANGE
  []
  [Variable_4]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[ICs]
  [Variable_3_IC]
    type = ConstantIC
    variable = Variable_3
    value = 3.0
  []
  [Variable_4_IC]
    type = ConstantIC
    variable = Variable_4
    value = 4.0
  []
[]

[UserObjects]
  [Variable_3_avg]
    type = NearestPointLayeredAverage
    direction = y
    num_layers = 10
    variable = Variable_3
    points = '0 0 0'
    execute_on = timestep_end
  []
  [Variable_4_avg]
    type = NearestPointLayeredAverage
    direction = y
    num_layers = 10
    variable = Variable_4
    points = '0 0 0'
    execute_on = timestep_end
  []
[]

[Problem]
  type = NoSolveProblem
[]

[Outputs]
  exodus = true
[]

[Executioner]
  type = Steady
  nl_rel_tol = 0.9
  l_tol = 0.9
[]
