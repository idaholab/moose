[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  xmax = 1
  ymax = 1
[]

[Variables]
  [u]
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

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[Problem]
  type = FEProblem
[]

[UserObjects]
  [message_out]
    type = MessageFromInput
    execute_on = timestep_end
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu superlu_dist'
[]

[Outputs]
  exodus = true
[]

