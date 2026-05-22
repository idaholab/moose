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
    convergence_names = 'nl0'
  []
  [sub1]
    type = NewtonSNESExecutor
    nonlinear_system_names = 'nl1'
    convergence_names = 'nl1'
  []
  [steady]
    type = SteadyExecutor
    inner_executors = 'outer'
  []
[]
