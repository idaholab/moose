# Abstraction groundwater model.  See groundwater_models.md for a detailed description
[Mesh]
  [from_steady_state]
    type = FileMeshGenerator
    file = gold/ex02_steady_state_ex.e
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [pp]
  []
[]

[ICs]
  [pp]
    type = FunctionIC
    variable = pp
    function = steady_state_pp
  []
[]

[BCs]
  [rainfall_recharge]
    type = PorousFlowSink
    boundary = zmax
    variable = pp
    flux_function = -1E-6  # recharge of 0.1mm/day = 1E-4m3/m2/day = 0.1kg/m2/day ~ 1E-6kg/m2/s
  []
  [evapotranspiration]
    type = PorousFlowHalfCubicSink
    boundary = zmax
    variable = pp
    center = 0.0
    cutoff = -5E4 # roots of depth 5m.  5m of water = 5E4 Pa
    use_mobility = true
    fluid_phase = 0
    # Assume pan evaporation of 4mm/day = 4E-3m3/m2/day = 4kg/m2/day ~ 4E-5kg/m2/s
    # Assume that if permeability was 1E-10m^2 and water table at topography then ET acts as pan strength
    # Because use_mobility = true, then 4E-5 = maximum_flux = max * perm * density / visc = max * 1E-4, so max = 40
    max = 40
  []
[]

[DiracKernels]
  inactive = polyline_sink_borehole
  [river]
    type = PorousFlowPolyLineSink
    SumQuantityUO = baseflow
    point_file = ex02_river.bh
    # Assume a perennial river.
    # Assume the river has an incision depth of 1m and a stage height of 1.5m, and these are constant in time and uniform over the whole model.  Hence, if groundwater head is 0.5m (5000Pa) there will be no baseflow and leakage.
    p_or_t_vals = '-999995000 5000 1000005000'
    # Assume the riverbed conductance, k_zz*density*river_segment_length*river_width/riverbed_thickness/viscosity = 1E-6*river_segment_length kg/Pa/s
    fluxes = '-1E3 0 1E3'
    variable = pp
  []
  [horizontal_borehole]
    type = PorousFlowPeacemanBorehole
    SumQuantityUO = abstraction
    bottom_p_or_t = -1E5
    unit_weight = '0 0 -1E4'
    character = 1.0
    point_file = ex02.bh
    variable = pp
  []
  [polyline_sink_borehole]
    type = PorousFlowPolyLineSink
    SumQuantityUO = abstraction
    fluxes = '-0.4 0 0.4'
    p_or_t_vals = '-1E8 0 1E8'
    point_file = ex02.bh
    variable = pp
  []
[]

[Functions]
  [steady_state_pp]
    type = SolutionFunction
    from_variable = pp
    solution = steady_state_solution
  []
  [baseflow_rate]
    type = ParsedFunction
    symbol_names = 'baseflow_kg dt'
    symbol_values = 'baseflow_kg dt'
    expression = 'baseflow_kg / dt * 24.0 * 3600.0 / 400.0'
  []
  [abstraction_rate]
    type = ParsedFunction
    symbol_names = 'abstraction_kg dt'
    symbol_values = 'abstraction_kg dt'
    expression = 'abstraction_kg / dt * 24.0 * 3600.0'
  []
[]

[AuxVariables]
  [ini_pp]
  []
  [pp_change]
  []
[]

[AuxKernels]
  [ini_pp]
    type = FunctionAux
    variable = ini_pp
    function = steady_state_pp
    execute_on = INITIAL
  []
  [pp_change]
    type = ParsedAux
    variable = pp_change
    coupled_variables = 'pp ini_pp'
    expression = 'pp - ini_pp'
  []
[]

[PorousFlowUnsaturated]
  fp = simple_fluid
  porepressure = pp
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
  []
[]

[Materials]
  [porosity_everywhere]
    type = PorousFlowPorosityConst
    porosity = 0.05
  []
  [permeability_aquifers]
    type = PorousFlowPermeabilityConst
    block = 'top_aquifer bot_aquifer'
    permeability = '1E-12 0 0 0 1E-12 0 0 0 1E-13'
  []
  [permeability_aquitard]
    type = PorousFlowPermeabilityConst
    block = aquitard
    permeability = '1E-16 0 0 0 1E-16 0 0 0 1E-17'
  []
[]

[UserObjects]
  [steady_state_solution]
    type = SolutionUserObject
    execute_on = INITIAL
    mesh = gold/ex02_steady_state_ex.e
    timestep = LATEST
    system_variables = pp
  []
  [baseflow]
    type = PorousFlowSumQuantity
  []
  [abstraction]
    type = PorousFlowSumQuantity
  []
[]

[Postprocessors]
  [baseflow_kg]
    type = PorousFlowPlotQuantity
    uo = baseflow
    outputs = 'none'
  []
  [dt]
    type = TimestepSize
    outputs = 'none'
  []
  [baseflow_l_per_m_per_day]
    type = FunctionValuePostprocessor
    function = baseflow_rate
    indirect_dependencies = 'baseflow_kg dt'
  []
  [abstraction_kg]
    type = PorousFlowPlotQuantity
    uo = abstraction
    outputs = 'none'
  []
  [abstraction_kg_per_day]
    type = FunctionValuePostprocessor
    function = abstraction_rate
    indirect_dependencies = 'abstraction_kg dt'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
    # following 2 lines are not mandatory, but illustrate a popular preconditioner choice in groundwater models
    petsc_options_iname = '-pc_type -sub_pc_type  -pc_asm_overlap'
    petsc_options_value = ' asm      ilu           2              '
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 100
  [TimeStepper]
    type = FunctionDT
    function = 'max(100, t)'
  []
  end_time = 8.64E5 # 10 days
  nl_abs_tol = 1E-11
[]

[Outputs]
  print_linear_residuals = false
  [ex]
    type = Exodus
    execute_on = final
  []
  [csv]
    type = CSV
  []
[]
