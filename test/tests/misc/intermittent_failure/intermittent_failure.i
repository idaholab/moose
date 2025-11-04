[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
[]

[Variables]
  [./u]
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
  num_steps = 20
  dt = 0.1
  solve_type = Newton
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

# This object will behave different on different invocations if
# MOOSE_ENABLE_INTERMITTENT_FAILURES is set
[UserObjects]
  [intermittent_failure]
    type = IntermittentFailureUO
    timestep_to_fail = 2
  []
[]
