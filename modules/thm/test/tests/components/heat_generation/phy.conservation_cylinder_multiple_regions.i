# Tests conservation for heat generation in a cylindrical heat structure with a
# heat source over multiple heat structure regions

[GlobalParams]
  scaling_factor_temperature = 1e-3
[]

[HeatStructureMaterials]
  [./main-material]
    type = SolidMaterialProperties
    k = 1e4
    Cp = 500.0
    rho = 100.0
  [../]
[]

[Functions]
  [./T0_fn]
    type = ParsedFunction
    value = '290 + 20 * (y - 1)'
  [../]
[]

[Components]
  [./heat_structure]
    type = HeatStructureCylindrical
    num_rods = 5

    position = '0 1 0'
    orientation = '1 0 0'
    length = 1.0
    n_elems = 100

    names = 'rgn1 rgn2 rgn3'
    materials = 'main-material main-material main-material'
    widths = '0.5 0.5 0.5'
    n_part_elems = '2 2 2'

    initial_T = T0_fn
  [../]
  [./heat_generation]
    type = HeatGeneration
    hs = heat_structure
    regions = 'rgn1 rgn2'
    power = reactor
  [../]
  [./reactor]
    type = PrescribedReactorPower
    function = 1e5
  [../]
[]

[Postprocessors]
  [./E_tot]
    type = HeatStructureEnergyRZ
    block = 'heat_structure:rgn1 heat_structure:rgn2 heat_structure:rgn3'
    n_units = 5
    execute_on = 'initial timestep_end'
  [../]
  [./E_tot_change]
    type = ChangeOverTimePostprocessor
    change_with_respect_to_initial = true
    postprocessor = E_tot
    execute_on = 'initial timestep_end'
  [../]
[]

[Preconditioning]
  [./pc]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  solve_type = 'PJFNK'
  line_search = 'basic'
  nl_rel_tol = 0
  nl_abs_tol = 1e-6
  nl_max_its = 15

  l_tol = 1e-3
  l_max_its = 10

  start_time = 0.0
  dt = 0.1
  num_steps = 10

  abort_on_solve_fail = true

  [./Quadrature]
    type = GAUSS
    order = SECOND
  [../]
[]

[Outputs]
  file_base = 'phy.conservation_cylinder_multiple_regions'
  csv = true
  show = 'E_tot_change'
  execute_on = 'final'
[]
