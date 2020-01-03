[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
[]

[Variables]
  [u]
  []
[]

[UserObjects]
  [all_sys]
    type = AllSystemsEvaluable
    execute_on = 'initial'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]

[Problem]
  type = MooseTestProblem
  solve = false
  kernel_coverage_check = false
[]
