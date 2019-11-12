[Mesh]
  type = GeneratedMesh
  dim = 2 # Problem dimension
  nx = 10
  ny = 10
[]

[GlobalParams]
  op_num = 2 # Number of grains
  var_name_base = gr # Base name of grains
[]

[AuxVariables]
  [./gr0]
    [./InitialCondition]
      type = FunctionIC
      function = x
    [../]
  [../]
  [./gr1]
    [./InitialCondition]
      type = FunctionIC
      function = y
    [../]
  [../]
[]

[Materials]
  [./Tester]
    type = EBCoupledVarTest
    outputs = exodus
  [../]
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[../]

[Outputs]
  exodus = true
  execute_on = 'INITIAL TIMESTEP_END'
[]
