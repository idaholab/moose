[Mesh]
  [./square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./u_aux]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]
  [./ts_func]
    type = TimestepSetupFunction
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./u_td]
    type = TimeDerivative
    variable = u
  [../]
[]

[AuxKernels]
  [./ts_aux]
    type = FunctionAux
    variable = u_aux
    function = ts_func
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'
  nl_rel_tol = 1e-10
  num_steps = 5
  dt = 1
[]

[Outputs]
  file_base = out
  exodus = true
[]
