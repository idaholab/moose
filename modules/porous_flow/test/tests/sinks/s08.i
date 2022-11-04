# apply a sink flux on just one component of a 3-component, 2-phase system and observe the correct behavior
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
    porous_flow_vars = 'pwater frac_ph0_c0 pgas'
    number_fluid_phases = 2
    number_fluid_components = 3
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    m = 0.5
    alpha = 1.1
  []
[]

[Variables]
  [pwater]
  []
  [frac_ph0_c0]
    initial_condition = 0.3
  []
  [pgas]
  []
[]

[ICs]
  [pwater]
    type = FunctionIC
    variable = pwater
    function = y
  []
  [pgas]
    type = FunctionIC
    variable = pgas
    function = y+3
  []
[]

[Kernels]
  [mass_c0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = frac_ph0_c0
  []
  [mass_c1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = pwater
  []
  [mass_c2]
    type = PorousFlowMassTimeDerivative
    fluid_component = 2
    variable = pgas
  []
[]

[FluidProperties]
  [simple_fluid0]
    type = SimpleFluidProperties
    bulk_modulus = 2.3
    density0 = 1.5
    thermal_expansion = 0
    viscosity = 2.1
  []
  [simple_fluid1]
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
    type = PorousFlow2PhasePP
    phase0_porepressure = pwater
    phase1_porepressure = pgas
    capillary_pressure = pc
  []
  [massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = 'frac_ph0_c0 frac_ph0_c1 frac_ph1_c0 frac_ph1_c1'
  []
  [simple_fluid0]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid0
    phase = 0
  []
  [simple_fluid1]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid1
    phase = 1
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '0.2 0 0 0 0.1 0 0 0 0.1'
  []
  [relperm0]
    type = PorousFlowRelativePermeabilityCorey
    n = 1
    phase = 0
  []
  [relperm1]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 1
  []
[]

[AuxVariables]
  [flux_out]
  []
  [frac_ph0_c1]
    initial_condition = 0.35
  []
  [frac_ph1_c0]
    initial_condition = 0.1
  []
  [frac_ph1_c1]
    initial_condition = 0.8
  []
[]

[Functions]
  [mass1_00]
    type = ParsedFunction
    expression = 'fgas*vol*por*dens0gas*exp(pgas/bulkgas)*(1-pow(1+pow(al*(pgas-pwater),1.0/(1-m)),-m))+fwater*vol*por*dens0water*exp(pwater/bulkwater)*(pow(1+pow(al*(pgas-pwater),1.0/(1-m)),-m))'
    symbol_names = 'vol  por dens0gas pgas    pwater    bulkgas al  m   dens0water bulkwater fgas           fwater'
    symbol_values = '0.25 0.1 1.1      pgas_00 pwater_00 1.3     1.1 0.5 1.5        2.3       frac_ph1_c1_00 frac_ph0_c1_00'
  []
  [expected_mass_change1_00]
    type = ParsedFunction
    expression = 'frac*fcn*area*dt*pow(1-pow(1+pow(al*(pgas-pwater),1.0/(1-m)),-m), 2)'
    symbol_names = 'frac           fcn area dt  pgas    pwater    al m'
    symbol_values = 'frac_ph1_c1_00 100 0.5 1E-3 pgas_00 pwater_00 1.1 0.5'
  []
  [mass1_00_expect]
    type = ParsedFunction
    expression = 'mass_prev-mass_change'
    symbol_names = 'mass_prev mass_change'
    symbol_values = 'm1_00_prev  del_m1_00'
  []
[]

[Postprocessors]
  [total_mass_comp0]
    type = PorousFlowFluidMass
    fluid_component = 0
  []
  [total_mass_comp1]
    type = PorousFlowFluidMass
    fluid_component = 1
  []
  [total_mass_comp2]
    type = PorousFlowFluidMass
    fluid_component = 2
  []
  [frac_ph1_c1_00]
    type = PointValue
    point = '0 0 0'
    variable = frac_ph1_c1
    execute_on = 'initial timestep_end'
  []
  [frac_ph0_c1_00]
    type = PointValue
    point = '0 0 0'
    variable = frac_ph0_c1
    execute_on = 'initial timestep_end'
  []
  [flux_00]
    type = PointValue
    point = '0 0 0'
    variable = flux_out
    execute_on = 'initial timestep_end'
  []
  [pgas_00]
    type = PointValue
    point = '0 0 0'
    variable = pgas
    execute_on = 'initial timestep_end'
  []
  [pwater_00]
    type = PointValue
    point = '0 0 0'
    variable = pwater
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
[]

[BCs]
  [flux_ph1_c1]
    type = PorousFlowSink
    boundary = 'left'
    variable = pwater # sink applied to the mass_c1 Kernel
    use_mobility = false
    use_relperm = true
    mass_fraction_component = 1
    fluid_phase = 1
    flux_function = 100
    save_in = flux_out
  []
[]

[Preconditioning]
  [andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -snes_max_it -sub_pc_factor_shift_type -pc_asm_overlap'
    petsc_options_value = 'gmres asm lu 100 NONZERO 2'
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
  file_base = s08
  exodus = true
  [console]
    type = Console
    execute_on = 'nonlinear linear'
  []
  [csv]
    type = CSV
    execute_on = 'timestep_end'
  []
[]
