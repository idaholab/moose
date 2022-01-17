[GlobalParams]
[]

[Components]
  [shaft]
    type = Shaft
    connected_components = ''
  []
[]

[Preconditioning]
  [SMP_PJFNK]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  start_time = 0
  dt = 1e-2
  num_steps = 1
  abort_on_solve_fail = true
[]
