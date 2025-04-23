# apply a half-cubic sink flux and observe the correct behavior
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
    porous_flow_vars = 'pp'
    number_fluid_phases = 1
    number_fluid_components = 1
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
[]

[ICs]
  [pp]
    type = FunctionIC
    variable = pp
    function = x*(y+1)
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
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
    permeability = '1E-5 0 0 0 1E-5 0 0 0 1E-5'
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
  [flux00]
    type = PointValue
    variable = flux_out
    point = '0 0 0'
  []
  [flux01]
    type = PointValue
    variable = flux_out
    point = '0 1 0'
  []
  [flux10]
    type = PointValue
    variable = flux_out
    point = '1 0 0'
  []
  [flux11]
    type = PointValue
    variable = flux_out
    point = '1 1 0'
  []
  [p00]
    type = PointValue
    point = '0 0 0'
    variable = pp
    execute_on = 'initial timestep_end'
  []
  [p10]
    type = PointValue
    point = '1 0 0'
    variable = pp
    execute_on = 'initial timestep_end'
  []
  [m10]
    type = ParsedPostprocessor
    expression = 'vol*por*dens0*exp(p10/bulk)*if(p10>=0,1,pow(1+pow(-al*p10,1.0/(1-m)),-m))'
    constant_names = 'vol por dens0 bulk al m'
    constant_expressions = '0.25 0.1 1.1 1.3 1.1 0.5'
    pp_names = 'p10'
    execute_on = 'initial timestep_end'
  []
  [dm10]
    type = ChangeOverTimePostprocessor
    postprocessor = m10
    outputs = none
  []
  [m10_prev]
    type = ParsedPostprocessor
    expression = 'm10 - dm10'
    pp_names = 'm10 dm10'
    outputs = 'console'
  []
  [m10_rate]
    type = ParsedPostprocessor
    expression = 'fcn*if(p10>center,m,if(p10<themin,0,m/c/c/c*(2*(p10-center)+c)*((p10-center)-c)*((p10-center)-c)))'
    constant_names = 'm fcn center sd themin c'
    constant_expressions = '2 3 0.9 0.5 0.1 -0.8'
    pp_names = 'p10'
  []
  [m10_expect]
    type = ParsedPostprocessor
    expression = 'm10_prev-m10_rate*area*dt'
    constant_names = 'area dt'
    constant_expressions = '0.5 2E-3'
    pp_names = 'm10_prev m10_rate'
  []
  [p01]
    type = PointValue
    point = '0 1 0'
    variable = pp
    execute_on = 'initial timestep_end'
  []
  [p11]
    type = PointValue
    point = '1 1 0'
    variable = pp
    execute_on = 'initial timestep_end'
  []
  [m11]
    type = ParsedPostprocessor
    expression = 'vol*por*dens0*exp(p11/bulk)*if(p11>=0,1,pow(1+pow(-al*p11,1.0/(1-m)),-m))'
    constant_names = 'vol por dens0 bulk al m'
    constant_expressions = '0.25 0.1 1.1 1.3 1.1 0.5'
    pp_names = 'p11'
    execute_on = 'initial timestep_end'
  []
  [dm11]
    type = ChangeOverTimePostprocessor
    postprocessor = m11
    outputs = none
  []
  [m11_prev]
    type = ParsedPostprocessor
    expression = 'm11 - dm11'
    pp_names = 'm11 dm11'
    outputs = 'console'
  []
  [m11_rate]
    type = ParsedPostprocessor
    expression = 'fcn*if(p11>center,m,if(p11<themin,0,m/c/c/c*(2*(p11-center)+c)*((p11-center)-c)*((p11-center)-c)))'
    constant_names = 'm fcn center sd themin c'
    constant_expressions = '2 3 0.9 0.5 0.1 -0.8'
    pp_names = 'p11'
  []
  [m11_expect]
    type = ParsedPostprocessor
    expression = 'm11_prev-m11_rate*area*dt'
    constant_names = 'area dt'
    constant_expressions = '0.5 2E-3'
    pp_names = 'm11_prev m11_rate'
  []
[]

[BCs]
  [flux]
    type = PorousFlowHalfCubicSink
    boundary = 'left right'
    max = 2
    cutoff = -0.8
    center = 0.9
    variable = pp
    use_mobility = false
    use_relperm = false
    fluid_phase = 0
    flux_function = 3
    save_in = flux_out
  []
[]

[Preconditioning]
  [andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -snes_max_it -sub_pc_factor_shift_type -pc_asm_overlap'
    petsc_options_value = 'gmres asm lu 10000 NONZERO 2'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 2E-3
  end_time = 6E-2
  nl_rel_tol = 1E-12
  nl_abs_tol = 1E-12
[]

[Outputs]
  file_base = s06
  [console]
    type = Console
    execute_on = 'nonlinear linear'
    time_step_interval = 5
  []
  [csv]
    type = CSV
    execute_on = 'timestep_end'
    time_step_interval = 3
  []
[]
