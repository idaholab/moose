[Components]
  [total_power]
    type = PrescribedReactorPower
    power = 1.
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

  dt = 1
  abort_on_solve_fail = true

  solve_type = 'PJFNK'
  line_search = 'basic'
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-6
  nl_max_its = 10

  l_tol = 1e-3
  l_max_its = 300

  start_time = 0.0
  end_time = 10
[]
