# ShellBlockGSSNESExecutor + NewtonSNESExecutor test.
# Two independent diffusion systems: nl0 and nl1.
# ShellBlockGSSNESExecutor sweeps both systems as the NPC for NewtonSNESExecutor,
# exercising the multi-system Case 3 (VecNest/MatNest) path.
# Both systems have the analytic solution u=v=x on [0,1].
# Run with --executor.

[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 1
  nx = 10
[]

[Problem]
  nl_sys_names = 'nl0 nl1'
[]

[Variables]
  [u]
    solver_sys = nl0
  []
  [v]
    solver_sys = nl1
  []
[]

[Kernels]
  [diff_u]
    type = Diffusion
    variable = u
  []
  [diff_v]
    type = Diffusion
    variable = v
  []
[]

[BCs]
  [left_u]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right_u]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
  [left_v]
    type = DirichletBC
    variable = v
    boundary = left
    value = 0
  []
  [right_v]
    type = DirichletBC
    variable = v
    boundary = right
    value = 1
  []
[]

[Executors]
  [shell_pc]
    type = ShellBlockGSSNESExecutor
    sub_snes_executors = 'sub0 sub1'
  []
  [outer]
    type = NewtonSNESExecutor
    nl_preconditioning = shell_pc
    nonlinear_system_names = 'nl0 nl1'
  []
  [sub0]
    type = NewtonSNESExecutor
    nonlinear_system_names = 'nl0'
  []
  [sub1]
    type = NewtonSNESExecutor
    nonlinear_system_names = 'nl1'
  []
[]

[Outputs]
  exodus = true
[]
