[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 32
  ny = 32
  uniform_refine = 2
[]

[Variables]
  [./phi]
  [../]
[]

[AuxVariables]
  [./phi_0]
  [../]
  [./marker]
  [../]
[]

[Kernels]
  [./time]
    type = TimeDerivative
    variable = phi
  [../]
  [./reinit]
    type = LevelSetOlssonReinitialization
    variable = phi
    phi_0 = phi_0
    epsilon = 0.03
  [../]
[]

[Problem]
  type = LevelSetReinitializationProblem
[]

[UserObjects]
  [./arnold]
    type = LevelSetOlssonTerminator
    tol = 1
    min_steps = 3
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  start_time = 0
  num_steps = 100
  nl_abs_tol = 1e-14
  scheme = crank-nicolson
  line_search = none
  petsc_options_iname = '-pc_type -pc_sub_type'
  petsc_options_value = 'asm      ilu'
  dt = 0.003
[]

[Outputs]
[]
