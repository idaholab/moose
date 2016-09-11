# apply a sink flux and observe the correct behavior
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
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp'
    number_fluid_phases = 1
    number_fluid_components = 1
  [../]
[]

[Variables]
  [./pp]
  [../]
[]

[ICs]
  [./pp]
    type = FunctionIC
    variable = pp
    function = y+1
  [../]
[]

[Kernels]
  [./mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  [../]
[]

[Materials]
  [./nnn]
    type = PorousFlowNodeNumber
    on_initial_only = true
  [../]
  [./temperature]
    type = PorousFlowTemperature
  [../]
  [./ppss]
    type = PorousFlow1PhaseP_VG
    porepressure = pp
    al = 1
    m = 0.5
  [../]
  [./massfrac]
    type = PorousFlowMassFraction
  [../]
  [./dens0]
    type = PorousFlowDensityConstBulk
    density_P0 = 1.1
    bulk_modulus = 1.3
    phase = 0
  [../]
  [./dens_all]
    type = PorousFlowJoiner
    include_old = true
    material_property = PorousFlow_fluid_phase_density
  [../]
  [./dens_all_at_quadpoints]
    type = PorousFlowJoiner
    material_property = PorousFlow_fluid_phase_density_qp
    at_qps = true
  [../]
  [./porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  [../]
  [./permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1E-5 0 0 0 1E-5 0 0 0 1E-5'
  [../]
  [./relperm]
    type = PorousFlowRelativePermeabilityCorey
    n_j = 2
    phase = 0
  [../]
  [./relperm_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_relative_permeability
  [../]
  [./visc0]
    type = PorousFlowViscosityConst
    viscosity = 1.1
    phase = 0
  [../]
  [./visc_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_viscosity
  [../]
[]

[AuxVariables]
  [./flux_out]
  [../]
  [./xval]
  [../]
  [./yval]
  [../]
[]

[ICs]
  [./xval]
    type = FunctionIC
    variable = xval
    function = x
  [../]
  [./yval]
    type = FunctionIC
    variable = yval
    function = y
  [../]
[]

[Functions]
  [./mass00]
    type = ParsedFunction
    value = 'vol*por*dens0*exp(pp/bulk)'
    vars = 'vol por dens0 pp bulk'
    vals = '0.25 0.1 1.1 p00 1.3'
  [../]
  [./mass01]
    type = ParsedFunction
    value = 'vol*por*dens0*exp(pp/bulk)'
    vars = 'vol por dens0 pp bulk'
    vals = '0.25 0.1 1.1 p01 1.3'
  [../]
  [./expected_mass_change00]
    type = ParsedFunction
    value = 'fcn*perm*dens0*exp(pp/bulk)/visc*area*dt'
    vars = 'fcn perm dens0 pp bulk visc area dt'
    vals = '6   1    1      0  1.3  1  0.5  1E-3'
  [../]
[]

[Postprocessors]
  [./p00]
    type = PointValue
    point = '0 0 0'
    variable = pp
    execute_on = 'initial timestep_end'
  [../]
  [./m00]
    type = FunctionValuePostprocessor
    function = mass00
    execute_on = 'initial timestep_end'
  [../]
  [./del_m00]
    type = FunctionValuePostprocessor
    function = expected_mass_change00
    execute_on = 'timestep_end'
  [../]
  [./p10]
    type = PointValue
    point = '1 0 0'
    variable = pp
    execute_on = 'initial timestep_end'
  [../]
  [./p01]
    type = PointValue
    point = '0 1 0'
    variable = pp
    execute_on = 'initial timestep_end'
  [../]
  [./m01]
    type = FunctionValuePostprocessor
    function = mass01
    execute_on = 'initial timestep_end'
  [../]
  [./p11]
    type = PointValue
    point = '1 1 0'
    variable = pp
    execute_on = 'initial timestep_end'
  [../]
[]

[BCs]
  [./flux]
    type = PorousFlowSink
    boundary = 'left'
    variable = pp
    use_mobility = false
    use_relperm = true
    fluid_phase = 0
    flux_function = 6
    save_in = flux_out
  [../]
[]

[Preconditioning]
  [./andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -snes_max_it -sub_pc_factor_shift_type -pc_asm_overlap'
    petsc_options_value = 'gmres asm lu 10000 NONZERO 2'
  [../]
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
  file_base = s01
  [./console]
    type = Console
    execute_on = 'nonlinear linear'
  [../]
  [./csv]
    type = CSV
    execute_on = 'initial timestep_end'
  [../]
[]
