[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 9
  ny = 9
[]

[Problem]
  kernel_coverage_check = false
  skip_nl_system_check = true
  solve = false
[]

[AuxVariables]
  [contains_point]
    order = CONSTANT
    family = MONOMIAL
    [AuxKernel]
      type = ContainsPointAux
      point = '.5 .5 0'
      execute_on = 'initial'
    []
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
