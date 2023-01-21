# Tests conservation for heat transfer between a cylindrical heat structure and
# a 1-phase flow channel

[GlobalParams]
  gravity_vector = '0 0 0'
  scaling_factor_1phase = '1e-3 1e-3 1e-8'
  scaling_factor_temperature = 1e-3
  closures = simple_closures
[]

[FluidProperties]
  [fp]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    q = -1167e3
    q_prime = 0
    p_inf = 1.e9
    cv = 1816
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[HeatStructureMaterials]
  [main-material]
    type = SolidMaterialProperties
    k = 1e4
    cp = 500.0
    rho = 100.0
  []
[]

[Functions]
  [T0_fn]
    type = ParsedFunction
    expression = '290 + 20 * (y - 1)'
  []
[]

[Components]
  [left_wall]
    type = SolidWall1Phase
    input = 'pipe:in'
  []

  [pipe]
    type = FlowChannel1Phase
    fp = fp

    position = '0 2 0'
    orientation = '1 0 0'
    length = 1.0
    n_elems = 5
    A = 1.0

    initial_T = 300
    initial_p = 1e5
    initial_vel = 0

    f = 0
  []

  [right_wall]
    type = SolidWall1Phase
    input = 'pipe:out'
  []

  [heat_transfer]
    type = HeatTransferFromHeatStructure1Phase
    flow_channel = pipe
    hs = heat_structure
    hs_side = inner
    Hw = 1e3
  []

  [heat_structure]
    #type = set externally
    num_rods = 5

    position = '0 2 0'
    orientation = '1 0 0'
    length = 1.0
    n_elems = 5

    names = 'main'
    materials = 'main-material'
    widths = '1.0'
    n_part_elems = '5'

    initial_T = T0_fn
  []
[]

[Postprocessors]
  [E_pipe]
    type = ElementIntegralVariablePostprocessor
    variable = rhoEA
    block = pipe
    execute_on = 'initial timestep_end'
  []
  [E_heat_structure]
    block = 'heat_structure:main'
    n_units = 5
    execute_on = 'initial timestep_end'
  []
  [E_tot]
    type = SumPostprocessor
    values = 'E_pipe E_heat_structure'
    execute_on = 'initial timestep_end'
  []
  [E_tot_change]
    type = ChangeOverTimePostprocessor
    change_with_respect_to_initial = true
    postprocessor = E_tot
    compute_relative_change = true
    execute_on = 'initial timestep_end'
  []
[]

[Preconditioning]
  [pc]
    type = SMP
    full = true
  []
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
  dt = 0.01
  num_steps = 5

  abort_on_solve_fail = true

  [Quadrature]
    type = GAUSS
    order = SECOND
  []
[]

[Outputs]
  file_base = 'phy.conservation_1phase_cylinder'
  csv = true
  show = 'E_tot_change'
  execute_on = 'final'
[]
