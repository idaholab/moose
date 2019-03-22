[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
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
  [./diff_dt]
    # should add 0 since the kernel is just \dot v
    type = ADCoupledTimeTest
    variable = u
    v = v
  [../]
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
  type = Transient
  steady_state_detection = true
  solve_type = 'Newton'
[]

[Outputs]
  exodus = true
[]
