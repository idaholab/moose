# Basic NonlinearPreconditioning (NLE) test.
# Two independent diffusion systems: nl0 (outer) and nl1 (inner/NPC).
# nl1 is solved as a nonlinear preconditioner before each outer Newton step.
# Both systems have the analytic solution u=v=x on [0,1].

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

[NonlinearPreconditioning]
  [ex]
    type = NGS
    inner_nl_sys_names = 'nl0 nl1'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  nl_abs_tol = 1e-10
[]

[Outputs]
  exodus = true
[]
