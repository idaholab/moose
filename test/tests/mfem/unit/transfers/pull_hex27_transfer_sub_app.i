[Mesh]
  type = CoupledMFEMMesh
  file = gold/simple-cube-hex27.e
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
  [sent_variable]
    family = LAGRANGE
    order = SECOND
  []
[]

[ICs]
  [set_variable]
    type = FunctionIC
    variable = sent_variable
    function = set_variable
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