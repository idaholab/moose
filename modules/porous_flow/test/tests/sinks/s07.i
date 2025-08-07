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
    type = ParsedPostprocessor
    expression = 'f1_00*vol*por*dens0*exp(p00/bulk)*pow(1+pow(-al*p00,1.0/(1-m)),-m)'
    constant_names = 'vol por dens0 bulk al m'
    constant_expressions = '0.25 0.1 1.1 1.3 1.1 0.5'
    pp_names = 'f1_00 p00'
    execute_on = 'initial timestep_end'
  []
  [dm1_00]
    type = ChangeOverTimePostprocessor
    postprocessor = m1_00
    outputs = none
  []
  [m1_00_prev]
    type = ParsedPostprocessor
    expression = 'm1_00 - dm1_00'
    pp_names = 'm1_00 dm1_00'
    outputs = 'console'
  []
  [del_m1_00]
    type = ParsedPostprocessor
    expression = 'f1_00*fcn*area*dt'
    constant_names = 'fcn area dt'
    constant_expressions = '6  0.5  1E-3'
    pp_names = 'f1_00'
    outputs = 'console'
  []
  [m1_00_expect]
    type = ParsedPostprocessor
    expression = 'm1_00_prev-del_m1_00'
    pp_names = 'm1_00_prev del_m1_00'
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
    type = ParsedPostprocessor
    expression = 'f1_01*vol*por*dens0*exp(p01/bulk)*pow(1+pow(-al*p01,1.0/(1-m)),-m)'
    constant_names = 'vol por dens0 bulk al m'
    constant_expressions = '0.25 0.1 1.1 1.3 1.1 0.5'
    pp_names = 'f1_01 p01'
    execute_on = 'initial timestep_end'
  []
  [dm1_01]
    type = ChangeOverTimePostprocessor
    postprocessor = m1_01
    outputs = none
  []
  [m1_01_prev]
    type = ParsedPostprocessor
    expression = 'm1_01 - dm1_01'
    pp_names = 'm1_01 dm1_01'
    outputs = 'console'
  []
  [del_m1_01]
    type = ParsedPostprocessor
    expression = 'f1_01*fcn*area*dt'
    constant_names = 'fcn area dt'
    constant_expressions = '6  0.5  1E-3'
    pp_names = 'f1_01'
    outputs = 'console'
  []
  [m1_01_expect]
    type = ParsedPostprocessor
    expression = 'm1_01_prev-del_m1_01'
    pp_names = 'm1_01_prev del_m1_01'
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
