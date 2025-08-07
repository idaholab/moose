[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[SolidProperties]
  [sp_ss316]
    type = ThermalSS316Properties
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
    position = '5.0 0 0'
    orientation = '1 0 0'
    length = 5.0
    n_elems = 5
    A = 1.0

    initial_T = 300
    initial_p = 1e5
    initial_vel = 0

    f = 0
    fp = fp
    closures = simple_closures

    scaling_factor_1phase = '1 1 1e-5'
  []

  [right_wall]
    type = SolidWall1Phase
    input = 'pipe:out'
  []

  [heat_transfer]
    type = HeatTransferFromHeatStructure1Phase
    flow_channel = pipe
    hs = heat_structure
    hs_boundary = heat_structure:region2:inner
    Hw = 1e3
  []

  [heat_structure]
    type = HeatStructureCylindrical
    position = '0 0 0'
    orientation = '1 0 0'
    length = '5.0 5.0'
    n_elems = '5 5'
    axial_region_names = 'region1 region2'

    names = 'main'
    solid_properties = 'sp_ss316'
    solid_properties_T_ref = '500'
    widths = '1.0'
    n_part_elems = '1'
    inner_radius = 1.0

    initial_T = 500

    scaling_factor_temperature = 1e-8
  []
[]

[Postprocessors]
  [T_avg]
    type = ElementAverageValue
    block = 'pipe'
    variable = T
    execute_on = 'INITIAL TIMESTEP_END'
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
  dt = 0.01
  num_steps = 1

  solve_type = NEWTON
  nl_rel_tol = 0
  nl_abs_tol = 1e-6
  nl_max_its = 15

  l_tol = 1e-3
  l_max_its = 10
[]

[Outputs]
  csv = true
[]
