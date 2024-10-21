# Cold water injection into one side of the fracture network, and production from the other side
injection_rate = 10 # kg/s

[Mesh]
  uniform_refine = 0
  [cluster34]
    type = FileMeshGenerator
    file = 'Cluster_34.exo'
  []
  [injection_node]
    type = BoundingBoxNodeSetGenerator
    input = cluster34
    bottom_left = '-1000 0 -1000'
    top_right = '1000 0.504 1000'
    new_boundary = injection_node
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 -9.81E-6' # Note the value, because of pressure_unit
[]

[Variables]
  [frac_P]
    scaling = 1E6
  []
  [frac_T]
    initial_condition = 473
  []
[]

[ICs]
  [frac_P]
    type = FunctionIC
    variable = frac_P
    function = insitu_pp
  []
[]

[PorousFlowFullySaturated]
  coupling_type = ThermoHydro
  porepressure = frac_P
  temperature = frac_T
  fp = water
  pressure_unit = MPa
[]

[Kernels]
  [toMatrix]
    type = PorousFlowHeatMassTransfer
    variable = frac_T
    v = transferred_matrix_T
    transfer_coefficient = heat_transfer_coefficient
    save_in = joules_per_s
  []
[]

[AuxVariables]
  [heat_transfer_coefficient]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 0.0
  []
  [transferred_matrix_T]
    initial_condition = 473
  []
  [joules_per_s]
  []
  [normal_dirn_x]
    family = MONOMIAL
    order = CONSTANT
  []
  [normal_dirn_y]
    family = MONOMIAL
    order = CONSTANT
  []
  [normal_dirn_z]
    family = MONOMIAL
    order = CONSTANT
  []
  [enclosing_element_normal_length]
    family = MONOMIAL
    order = CONSTANT
  []
  [enclosing_element_normal_thermal_cond]
    family = MONOMIAL
    order = CONSTANT
  []
  [aperture]
    family = MONOMIAL
    order = CONSTANT
  []
  [perm_times_app]
    family = MONOMIAL
    order = CONSTANT
  []
  [density]
    family = MONOMIAL
    order = CONSTANT
  []
  [viscosity]
    family = MONOMIAL
    order = CONSTANT
  []
  [insitu_pp]
  []
[]

[AuxKernels]
  [normal_dirn_x_auxk]
    type = PorousFlowElementNormal
    variable = normal_dirn_x
    component = x
  []
  [normal_dirn_y]
    type = PorousFlowElementNormal
    variable = normal_dirn_y
    component = y
  []
  [normal_dirn_z]
    type = PorousFlowElementNormal
    variable = normal_dirn_z
    component = z
  []
  [heat_transfer_coefficient_auxk]
    type = ParsedAux
    variable = heat_transfer_coefficient
    coupled_variables = 'enclosing_element_normal_length enclosing_element_normal_thermal_cond'
    constant_names = h_s
    constant_expressions = 1E3 # should be much bigger than thermal_conductivity / L ~ 1
    expression = 'if(enclosing_element_normal_length = 0, 0, h_s * enclosing_element_normal_thermal_cond * 2 * enclosing_element_normal_length / (h_s * enclosing_element_normal_length * enclosing_element_normal_length + enclosing_element_normal_thermal_cond * 2 * enclosing_element_normal_length))'
  []
  [aperture]
    type = PorousFlowPropertyAux
    variable = aperture
    property = porosity
  []
  [perm_times_app]
    type = PorousFlowPropertyAux
    variable = perm_times_app
    property = permeability
    row = 0
    column = 0
  []
  [density]
    type = PorousFlowPropertyAux
    variable = density
    property = density
    phase = 0
  []
  [viscosity]
    type = PorousFlowPropertyAux
    variable = viscosity
    property = viscosity
    phase = 0
  []
  [insitu_pp]
    type = FunctionAux
    execute_on = initial
    variable = insitu_pp
    function = insitu_pp
  []
[]

[BCs]
  [inject_heat]
    type = DirichletBC
    boundary = injection_node
    variable = frac_T
    value = 373
  []
[]

[DiracKernels]
  [inject_fluid]
    type = PorousFlowPointSourceFromPostprocessor
    mass_flux = ${injection_rate}
    point = '58.8124 0.50384 74.7838'
    variable = frac_P
  []
  [withdraw_fluid]
    type = PorousFlowPeacemanBorehole
    SumQuantityUO = kg_out_uo
    bottom_p_or_t = 10.6 # 1MPa + approx insitu at production point, to prevent aperture closing due to low porepressures
    character = 1
    line_length = 1
    point_file = production.xyz
    unit_weight = '0 0 0'
    fluid_phase = 0
    use_mobility = true
    variable = frac_P
  []
  [withdraw_heat]
    type = PorousFlowPeacemanBorehole
    SumQuantityUO = J_out_uo
    bottom_p_or_t = 10.6 # 1MPa + approx insitu at production point, to prevent aperture closing due to low porepressures
    character = 1
    line_length = 1
    point_file = production.xyz
    unit_weight = '0 0 0'
    fluid_phase = 0
    use_mobility = true
    use_enthalpy = true
    variable = frac_T
  []
[]

[UserObjects]
  [kg_out_uo]
    type = PorousFlowSumQuantity
  []
  [J_out_uo]
    type = PorousFlowSumQuantity
  []
[]

[FluidProperties]
  [true_water]
    type = Water97FluidProperties
  []
  [water]
    type = TabulatedBicubicFluidProperties
    fp = true_water
    temperature_min = 275 # K
    temperature_max = 600
    interpolated_properties = 'density viscosity enthalpy internal_energy'
    fluid_property_output_file = water97_tabulated.csv
    # Comment out the fp parameter and uncomment below to use the newly generated tabulation
    # fluid_property_file = water97_tabulated.csv
  []
[]

[Materials]
  [porosity]
    type = PorousFlowPorosityLinear
    porosity_ref = 1E-4 # fracture porosity = 1.0, but must include fracture aperture of 1E-4 at P = insitu_pp
    P_ref = insitu_pp
    P_coeff = 1E-3 # this is in metres/MPa, ie for P_ref = 1/P_coeff, the aperture becomes 1 metre
    porosity_min = 1E-5
  []
  [permeability]
    type = PorousFlowPermeabilityKozenyCarman
    k0 = 1E-15 # fracture perm = 1E-11 m^2, but must include fracture aperture of 1E-4
    poroperm_function = kozeny_carman_phi0
    m = 0
    n = 3
    phi0 = 1E-4
  []
  [internal_energy]
    type = PorousFlowMatrixInternalEnergy
    density = 2700 # kg/m^3
    specific_heat_capacity = 0 # basically no rock inside the fracture
  []
  [aq_thermal_conductivity]
    type = PorousFlowThermalConductivityIdeal
    dry_thermal_conductivity = '0.6E-4 0 0  0 0.6E-4 0  0 0 0.6E-4' # thermal conductivity of water times fracture aperture.  This should increase linearly with aperture, but is set constant in this model
  []
[]

[Functions]
  [kg_rate]
    type = ParsedFunction
    symbol_values = 'dt kg_out'
    symbol_names = 'dt kg_out'
    expression = 'kg_out/dt'
  []
  [insitu_pp]
    type = ParsedFunction
    expression = '10 - 0.847E-2 * z' # Approximate hydrostatic in MPa
  []
[]

[Postprocessors]
  [dt]
    type = TimestepSize
    outputs = 'none'
  []
  [kg_out]
    type = PorousFlowPlotQuantity
    uo = kg_out_uo
  []
  [kg_per_s]
    type = FunctionValuePostprocessor
    function = kg_rate
  []
  [J_out]
    type = PorousFlowPlotQuantity
    uo = J_out_uo
  []
  [TK_out]
    type = PointValue
    variable = frac_T
    point = '101.705 160.459 39.5722'
  []
  [P_out]
    type = PointValue
    variable = frac_P
    point = '101.705 160.459 39.5722'
  []
  [P_in]
    type = PointValue
    variable = frac_P
    point = '58.8124 0.50384 74.7838'
  []
[]

[VectorPostprocessors]
  [heat_transfer_rate]
    type = NodalValueSampler
    outputs = none
    sort_by = id
    variable = joules_per_s
  []
[]

[Preconditioning]
  [entire_jacobian]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap'
    petsc_options_value = ' asm      lu           NONZERO                   2             '
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1
    optimal_iterations = 10
    growth_factor = 1.5
  []
  dtmax = 1E8
  end_time = 1E8
  nl_abs_tol = 1E-3
  nl_max_its = 20
[]

[Outputs]
  print_linear_residuals = false
  csv = true
  [ex]
    type = Exodus
    sync_times = '1 10 100 200 300 400 500 600 700 800 900 1000 1100 1200 1300 1400 1500 1600 1700 1800 1900 2000 2100 2200 2300 2400 2500 2600 2700 2800 2900 3000 3100 3200 3300 3400 3500 3600 3700 3800 3900 4000 4100 4200 4300 4400 4500 4600 4700 4800 4900 5000 5100 5200 5300 5400 5500 5600 5700 5800 5900 6000 6100 6200 6300 6400 6500 6600 6700 6800 6900 7000 7100 7200 7300 7400 7500 7600 7700 7800 7900 8000 8100 8200 8300 8400 8500 8600 8700 8800 8900 9000 10000 11000 12000 13000 14000 15000 16000 17000 18000 19000 20000 30000 50000 70000 100000 200000 300000 400000 500000 600000 700000 800000 900000 1000000 1100000 1200000 1300000 1400000 1500000 1600000 1700000 1800000 1900000 2000000 2100000 2200000 2300000 2400000 2500000 2600000 2700000 2800000 2900000'
    sync_only = true
  []
[]
