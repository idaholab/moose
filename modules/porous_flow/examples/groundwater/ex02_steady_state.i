# Steady-state groundwater model.  See groundwater_models.md for a detailed description
[Mesh]
  [basic_mesh]
    # mesh create by external program: lies within -500<=x<=500 and -200<=y<=200, with varying z
    type = FileMeshGenerator
    file = ex02_mesh.e
  []
  [name_blocks]
    type = RenameBlockGenerator
    input = basic_mesh
    old_block = '2 3 4'
    new_block = 'bot_aquifer aquitard top_aquifer'
  []
  [zmax]
    type = SideSetsFromNormalsGenerator
    input = name_blocks
    new_boundary = zmax
    normals = '0 0 1'
  []
  [xmin_bot_aquifer]
    type = ParsedGenerateSideset
    input = zmax
    included_subdomain_ids = 2
    normal = '-1 0 0'
    combinatorial_geometry = 'x <= -500.0'
    new_sideset_name = xmin_bot_aquifer
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
    function = initial_pp
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
[]

[Functions]
  [initial_pp]
    type = SolutionFunction
    scale_factor = 1E4
    from_variable = cosflow_depth
    solution = initial_mesh
  []
  [baseflow_rate]
    type = ParsedFunction
    symbol_names = 'baseflow_kg dt'
    symbol_values = 'baseflow_kg dt'
    expression = 'baseflow_kg / dt * 24.0 * 3600.0 / 400.0'
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
  [initial_mesh]
    type = SolutionUserObject
    execute_on = INITIAL
    mesh = ex02_mesh.e
    timestep = LATEST
    system_variables = cosflow_depth
  []
  [baseflow]
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
  dt = 1E6
  [TimeStepper]
    type = FunctionDT
    function = 'max(1E6, t)'
  []
  end_time = 1E12
  nl_abs_tol = 1E-13
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
