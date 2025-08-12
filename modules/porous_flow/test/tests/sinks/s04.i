# apply a piecewise-linear sink flux and observe the correct behavior
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
    alpha = 1
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
    function = y+1
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
  [xval]
  []
  [yval]
  []
  [pt_shift]
    initial_condition = 0.3
  []
[]

[ICs]
  [xval]
    type = FunctionIC
    variable = xval
    function = x
  []
  [yval]
    type = FunctionIC
    variable = yval
    function = y
  []
[]

[Postprocessors]
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
    expression = 'vol*por*dens0*exp(p10/bulk)'
    constant_names = 'vol por dens0 bulk'
    constant_expressions = '0.25 0.1 1.1 1.3'
    pp_names = p10
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
    expression = 'fcn*if(p10>0.8,1,if(p10<0.3,0.5,0.2+p10))'
    constant_names = 'fcn'
    constant_expressions = '8'
    pp_names = 'p10'
  []
  [m10_expect]
    type = ParsedPostprocessor
    expression = 'm10_prev-m10_rate*area*dt'
    constant_names = 'area dt'
    constant_expressions = '0.5 1E-3'
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
    expression = 'vol*por*dens0*exp(p11/bulk)'
    constant_names = 'vol por dens0 bulk'
    constant_expressions = '0.25 0.1 1.1 1.3'
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
    expression = 'fcn*if(p11>0.8,1,if(p11<0.3,0.5,0.2+p11))'
    constant_names = 'fcn'
    constant_expressions = '8'
    pp_names = 'p11'
  []
  [m11_expect]
    type = ParsedPostprocessor
    expression = 'm11_prev-m11_rate*area*dt'
    constant_names = 'area dt'
    constant_expressions = '0.5 1E-3'
    pp_names = 'm11_prev m11_rate'
  []
[]

[BCs]
  [flux]
    type = PorousFlowPiecewiseLinearSink
    boundary = 'right'
    PT_shift = pt_shift
    pt_vals = '0.0 0.5'
    multipliers = '0.5 1'
    variable = pp
    use_mobility = false
    use_relperm = false
    fluid_phase = 0
    flux_function = 8
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
  dt = 1E-3
  end_time = 1E-2
  nl_rel_tol = 1E-12
  nl_abs_tol = 1E-12
[]

[Outputs]
  file_base = s04
  [console]
    type = Console
    execute_on = 'nonlinear linear'
  []
  [csv]
    type = CSV
    execute_on = 'timestep_end'
  []
[]
