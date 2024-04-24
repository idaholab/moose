[Mesh]
  type = CoupledMFEMMesh
  file = gold/simple-cube-multi-element-order1.e
  dim = 3
[]

[Problem]
  solve = false
  type = MFEMProblem
[]

[Formulation]
  type = CustomFormulation
[]

[AuxVariables]
  [received_variable_subapp]
    family = LAGRANGE
    order = FIRST
  []
[]

[Functions]
  [set_variable]
    type = ParsedFunction
    expression = '42 + 100*x*x'
  []
[]

[Executioner]
  type = Transient
  dt = 1.0
  start_time = 0.0
  end_time = 1.0
[]