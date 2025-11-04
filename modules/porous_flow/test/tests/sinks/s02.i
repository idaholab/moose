# apply a sink flux with use_mobility=true and observe the correct behavior
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
  [xval]
  []
  [yval]
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
  [m00]
    type = ParsedPostprocessor
    expression = 'vol*por*dens0*exp(p00/bulk)'
    constant_names = 'vol por dens0 bulk'
    constant_expressions = '0.25 0.1 1.1 1.3'
    pp_names = 'p00'
    execute_on = 'initial timestep_end'
  []
  [dm00]
    type = ChangeOverTimePostprocessor
    postprocessor = m00
    outputs = none
  []
  [m00_prev]
    type = ParsedPostprocessor
    expression = 'm00 - dm00'
    pp_names = 'm00 dm00'
    outputs = 'console'
  []
  [del_m00]
    type = ParsedPostprocessor
    expression = 'fcn*perm*dens0*exp(p00/bulk)/visc*area*dt'
    constant_names = 'fcn perm dens0 bulk visc area dt'
    constant_expressions = '6   0.2  1.1 1.3  1.1  0.5  1E-3'
    pp_names = 'p00'
    outputs = 'console'
  []
  [m00_expect]
    type = ParsedPostprocessor
    expression = 'm00_prev - del_m00'
    pp_names = 'm00_prev del_m00'
  []
  [p10]
    type = PointValue
    point = '1 0 0'
    variable = pp
    execute_on = 'initial timestep_end'
  []
  [p01]
    type = PointValue
    point = '0 1 0'
    variable = pp
    execute_on = 'initial timestep_end'
  []
  [m01]
    type = ParsedPostprocessor
    expression = 'vol*por*dens0*exp(p01/bulk)'
    constant_names = 'vol por dens0 bulk'
    constant_expressions = '0.25 0.1 1.1 1.3'
    pp_names = 'p01'
    execute_on = 'initial timestep_end'
  []
  [dm01]
    type = ChangeOverTimePostprocessor
    postprocessor = m01
    outputs = none
  []
  [m01_prev]
    type = ParsedPostprocessor
    expression = 'm01 - dm01'
    pp_names = 'm01 dm01'
    outputs = 'console'
  []
  [del_m01]
    type = ParsedPostprocessor
    expression = 'fcn*perm*dens0*exp(p01/bulk)/visc*area*dt'
    constant_names = 'fcn perm dens0 bulk visc area dt'
    constant_expressions = '6   0.2  1.1 1.3  1.1  0.5  1E-3'
    pp_names = 'p01'
    outputs = 'console'
  []
  [m01_expect]
    type = ParsedPostprocessor
    expression = 'm01_prev - del_m01'
    pp_names = 'm01_prev del_m01'
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
    variable = pp
    use_mobility = true
    use_relperm = true
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
    petsc_options_value = 'gmres asm lu 10000 NONZERO 2'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1E-3
  end_time = 0.03
  nl_rel_tol = 1E-12
  nl_abs_tol = 1E-12
[]

[Outputs]
  file_base = s02
  [console]
    type = Console
    execute_on = 'nonlinear linear'
    time_step_interval = 30
  []
  [csv]
    type = CSV
    execute_on = 'timestep_end'
    time_step_interval = 3
  []
[]
