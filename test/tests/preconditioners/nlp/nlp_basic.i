# Basic NGSSNESExecutor + NewtonSNESExecutor test.
# Two independent diffusion systems: nl0 and nl1.
# NGSSNESExecutor sweeps both systems as the NPC for NewtonSNESExecutor.
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
  [ngs_pc]
    type = NGSSNESExecutor
    inner_nl_sys_names = 'nl0 nl1'
  []
  [newton]
    type = NewtonSNESExecutor
    nl_preconditioning = ngs_pc
  []
[]

[Outputs]
  exodus = true
[]
