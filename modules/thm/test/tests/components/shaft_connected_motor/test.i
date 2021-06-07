[HeatStructureMaterials]
  [mat]
    type = SolidMaterialProperties
    rho = 1
    cp = 1
    k = 1
  []
[]

[Components]
  [motor]
    type = ShaftConnectedMotor
    inertia = 1
    torque = 2
  []

  [shaft]
    type = Shaft
    connected_components = 'motor'
  []

  [hs]
    type = HeatStructureCylindrical
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 1

    names = '0'
    n_part_elems = 1
    widths = '1'
    materials = 'mat'
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
  num_steps = 5
  abort_on_solve_fail = true

  solve_type = 'PJFNK'
  line_search = 'basic'
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-6
  nl_max_its = 15

  l_tol = 1e-4
  l_max_its = 10
[]

[Outputs]
  csv = true
  show = 'shaft:omega'
[]
