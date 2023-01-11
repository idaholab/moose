[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
[]

[AuxVariables]
  [vec]
    family = LAGRANGE_VEC
    order = FIRST
  []
[]

[Variables]
  [u][]
[]

[Functions]
  [function]
    type = ParsedVectorFunction
    expression_x = t*x
    expression_y = t*y
  []
[]

[AuxKernels]
  [vec]
    type = VectorFunctionAux
    variable = vec
    function = function
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
[]

[Problem]
  type = FEProblem
  #solve = false
  kernel_coverage_check = false
[]

[Executioner]
  type = Transient
  start_time = 0.0
  num_steps = 5
  dt = 1
[]

[Outputs]
  exodus = true
[]
