[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./aux0]
  [../]
  [./aux1]
  [../]
[]

[Functions]
  [./func]
    type = ParsedFunction
    expression = t*x*y
  [../]
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  [../]
  [./time]
    type = TimeDerivative
    variable = u
  [../]
[]

[AuxKernels]
  [./aux0]
    type = FunctionAux
    variable = aux0
    function = func
  [../]
  [./aux1]
    type = FunctionAux
    variable = aux1
    function = func
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
  num_steps = 10
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

[Controls]
  [./damping_control]
    type = TimePeriod
    disable_objects    = 'AuxKernels::aux0 AuxKernels::aux1'
    start_time         = '0.25             0.55'
    end_time           = '0.65             0.75'
    execute_on         = 'initial timestep_begin'
  [../]
[]
