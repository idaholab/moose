# INB stands for inexact Newton backtracking

!include physics.i

[Executors]
  [newton]
    type = NewtonSNESExecutor
    nonlinear_system_names = 'nl0'
  []
  [steady]
    type = SteadyExecutor
    inner_executors = 'newton'
  []
[]
