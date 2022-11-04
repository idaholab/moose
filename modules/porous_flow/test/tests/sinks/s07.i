# apply a sink flux on just one component of a 3-component system and observe the correct behavior
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  zmin = 0
  zmax = 2
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp frac0 frac1'
    number_fluid_phases = 1
    number_fluid_components = 3
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    m = 0.5
    alpha = 1.1
  []
[]

[Variables]
  [pp]
  []
  [frac0]
    initial_condition = 0.1
  []
  [frac1]
    initial_condition = 0.6
  []
[]

[ICs]
  [pp]
    type = FunctionIC
    variable = pp
    function = y
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = frac0
  []
  [mass1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = frac1
  []
  [mass2]
    type = PorousFlowMassTimeDerivative
    fluid_component = 2
    variable = pp
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 1.3
    density0 = 1.1
    thermal_expansion = 0
    viscosity = 1.1
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
  []
  [ppss]
    type = PorousFlow1PhaseP
    porepressure = pp
    capillary_pressure = pc
  []
  [massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = 'frac0 frac1'
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '0.2 0 0 0 0.1 0 0 0 0.1'
  []
  [relperm]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
  []
[]

[AuxVariables]
  [flux_out]
  []
[]

[Functions]
  [mass1_00]
    type = ParsedFunction
    expression = 'frac*vol*por*dens0*exp(pp/bulk)*pow(1+pow(-al*pp,1.0/(1-m)),-m)'
    symbol_names = 'frac  vol  por dens0 pp bulk al m'
    symbol_values = 'f1_00 0.25 0.1 1.1  p00 1.3 1.1 0.5'
  []
  [expected_mass_change1_00]
    type = ParsedFunction
    expression = 'frac*fcn*area*dt'
    symbol_names = 'frac fcn area dt'
    symbol_values = 'f1_00 6  0.5  1E-3'
  []
  [mass1_00_expect]
    type = ParsedFunction
    expression = 'mass_prev-mass_change'
    symbol_names = 'mass_prev mass_change'
    symbol_values = 'm1_00_prev  del_m1_00'
  []
  [mass1_01]
    type = ParsedFunction
    expression = 'frac*vol*por*dens0*exp(pp/bulk)*pow(1+pow(-al*pp,1.0/(1-m)),-m)'
    symbol_names = 'frac  vol  por dens0 pp bulk al m'
    symbol_values = 'f1_01 0.25 0.1 1.1  p01 1.3 1.1 0.5'
  []
  [expected_mass_change1_01]
    type = ParsedFunction
    expression = 'frac*fcn*area*dt'
    symbol_names = 'frac fcn area dt'
    symbol_values = 'f1_01 6  0.5  1E-3'
  []
  [mass1_01_expect]
    type = ParsedFunction
    expression = 'mass_prev-mass_change'
    symbol_names = 'mass_prev mass_change'
    symbol_values = 'm1_01_prev  del_m1_01'
  []
[]

[Postprocessors]
  [f1_00]
    type = PointValue
    point = '0 0 0'
    variable = frac1
    execute_on = 'initial timestep_end'
  []
  [flux_00]
    type = PointValue
    point = '0 0 0'
    variable = flux_out
    execute_on = 'initial timestep_end'
  []
  [p00]
    type = PointValue
    point = '0 0 0'
    variable = pp
    execute_on = 'initial timestep_end'
  []
  [m1_00]
    type = FunctionValuePostprocessor
    function = mass1_00
    execute_on = 'initial timestep_end'
  []
  [m1_00_prev]
    type = FunctionValuePostprocessor
    function = mass1_00
    execute_on = 'timestep_begin'
    outputs = 'console'
  []
  [del_m1_00]
    type = FunctionValuePostprocessor
    function = expected_mass_change1_00
    execute_on = 'timestep_end'
    outputs = 'console'
  []
  [m1_00_expect]
    type = FunctionValuePostprocessor
    function = mass1_00_expect
    execute_on = 'timestep_end'
  []
  [f1_01]
    type = PointValue
    point = '0 1 0'
    variable = frac1
    execute_on = 'initial timestep_end'
  []
  [flux_01]
    type = PointValue
    point = '0 1 0'
    variable = flux_out
    execute_on = 'initial timestep_end'
  []
  [p01]
    type = PointValue
    point = '0 1 0'
    variable = pp
    execute_on = 'initial timestep_end'
  []
  [m1_01]
    type = FunctionValuePostprocessor
    function = mass1_01
    execute_on = 'initial timestep_end'
  []
  [m1_01_prev]
    type = FunctionValuePostprocessor
    function = mass1_01
    execute_on = 'timestep_begin'
    outputs = 'console'
  []
  [del_m1_01]
    type = FunctionValuePostprocessor
    function = expected_mass_change1_01
    execute_on = 'timestep_end'
    outputs = 'console'
  []
  [m1_01_expect]
    type = FunctionValuePostprocessor
    function = mass1_01_expect
    execute_on = 'timestep_end'
  []
  [f1_11]
    type = PointValue
    point = '1 1 0'
    variable = frac1
    execute_on = 'initial timestep_end'
  []
  [flux_11]
    type = PointValue
    point = '1 1 0'
    variable = flux_out
    execute_on = 'initial timestep_end'
  []
  [p11]
    type = PointValue
    point = '1 1 0'
    variable = pp
    execute_on = 'initial timestep_end'
  []
[]

[BCs]
  [flux]
    type = PorousFlowSink
    boundary = 'left'
    variable = frac1
    use_mobility = false
    use_relperm = false
    mass_fraction_component = 1
    fluid_phase = 0
    flux_function = 6
    save_in = flux_out
  []
[]

[Preconditioning]
  [andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -snes_max_it -sub_pc_factor_shift_type -pc_asm_overlap'
    petsc_options_value = 'gmres asm lu 10 NONZERO 2'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1E-3
  end_time = 0.01
  nl_rel_tol = 1E-12
  nl_abs_tol = 1E-12
[]

[Outputs]
  file_base = s07
  [console]
    type = Console
    execute_on = 'nonlinear linear'
  []
  [csv]
    type = CSV
    execute_on = 'timestep_end'
  []
[]
