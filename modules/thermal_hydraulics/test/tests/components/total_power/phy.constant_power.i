[HeatStructureMaterials]
  [mat]
    type = SolidMaterialProperties
    k = 1
    cp = 1
    rho = 1
  []
[]

[Components]
  [total_power]
    type = TotalPower
    power = 1234.
  []

  [ch1:solid]
    type = HeatStructureCylindrical
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 1
    initial_T = 300
    names = '0'
    widths = '1'
    n_part_elems = '1'
    materials = 'mat'
  []
[]

[Postprocessors]
  [reactor_power]
    type = RealComponentParameterValuePostprocessor
    component = total_power
    parameter = power
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

[Outputs]
  csv = true
  show = 'reactor_power'
[]
