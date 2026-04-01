[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 1
  xmax = 1
  ymax = 1
[]

[UserObjects]
  [initial]
    type = WarningUserObject
    execute_on = INITIAL
  []
  [tbegin]
    type = WarningUserObject
    execute_on = TIMESTEP_BEGIN
  []
  [jacobian]
    type = WarningUserObject
    execute_on = NONLINEAR
  []
  [residual]
    type = WarningUserObject
    execute_on = LINEAR
  []
  [tend]
    type = WarningUserObject
    execute_on = TIMESTEP_END
  []
  [final]
    type = WarningUserObject
    execute_on = FINAL
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diffusion]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 1
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 0
  []
[]

[Problem]
  type = FEProblem
[]

[Executioner]
  # type = Steady
  type = Transient
  num_steps = 2
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu superlu_dist'

  nl_abs_tol = 1e-10
[]

[Reporters]
  [solution_invalidity]
    type = SolutionInvalidityReporter
  []
[]

[Outputs]
  [out]
    type = JSON
    execute_on = 'FINAL'
    execute_system_information_on = none
  []
[]
