[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [v]
    initial_condition = 2
  []
  [exact]
  []
[]

[ICs]
  [exact]
    type = FunctionIC
    function = 'x*(2-x)'
    variable = exact
  []
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./ad_coupled_value]
    type = ADCoupledValueTest
    variable = u
    v = v
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'Newton'
[]

[Outputs]
  exodus = true
[]
