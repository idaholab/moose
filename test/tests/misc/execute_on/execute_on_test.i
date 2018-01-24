[Mesh]
  type = GeneratedMesh
  dim = 2
  xmax = 1
  ymax = 1
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
[]

[AuxVariables]
  [./initial]
  [../]
  [./timestep_begin]
  [../]
  [./timestep_end]
  [../]
  [./nonlinear]
  [../]
  [./linear]
  [../]
  [./custom]
    initial_condition = 999
  [../]
[]

[AuxKernels]
  [./initial]
    type = CheckCurrentExecAux
    variable = initial
    execute_on = initial
  [../]
  [./timestep_begin]
    type = CheckCurrentExecAux
    variable = timestep_begin
    execute_on = timestep_begin
  [../]
  [./timestep_end]
    type = CheckCurrentExecAux
    variable = timestep_end
    execute_on = timestep_end
  [../]
  [./nonlinear]
    type = CheckCurrentExecAux
    variable = nonlinear
    execute_on = nonlinear
  [../]
  [./linear]
    type = CheckCurrentExecAux
    variable = linear
    execute_on = linear
  [../]
  [./custom]
    # this kernel will be never executed by Steady, so the initial value of this variable is not going to change
    type = CheckCurrentExecAux
    variable = custom
    execute_on = custom
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
