[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
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
    type = ADDiffusion
    variable = u
  []
  [diff_v]
    type = ADDiffusion
    variable = v
  []
[]

[Executors]
  [outer]
    type = NewtonSNESExecutor
    nonlinear_system_names = 'nl0 nl1'
  []
  [steady]
    type = SteadyExecutor
    inner_executors = 'outer'
  []
[]
