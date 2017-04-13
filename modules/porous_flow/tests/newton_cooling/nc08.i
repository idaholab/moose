# Newton cooling from a bar.  1-phase ideal fluid and heat, steady
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 100
  ny = 1
  xmin = 0
  xmax = 100
  ymin = 0
  ymax = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pressure temp'
    number_fluid_phases = 1
    number_fluid_components = 1
  [../]
[]

[Variables]
  [./pressure]
  [../]
  [./temp]
  [../]
[]

[ICs]
  # have to start these reasonably close to their steady-state values
  [./pressure]
    type = FunctionIC
    variable = pressure
    function = '200-0.5*x'
  [../]
  [./temperature]
    type = FunctionIC
    variable = temp
    function = 180+0.1*x
  [../]
[]

[Kernels]
  [./flux]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    gravity = '0 0 0'
    variable = pressure
  [../]
  [./heat_advection]
    type = PorousFlowHeatAdvection
    gravity = '0 0 0'
    variable = temp
  [../]
[]

[Materials]
  [./temperature]
    type = PorousFlowTemperature
    at_nodes = true
    temperature = temp
  [../]
  [./temperature_qp]
    type = PorousFlowTemperature
    temperature = temp
  [../]
  [./ppss]
    type = PorousFlow1PhaseP_VG # irrelevant in this fully-saturated situation
    porepressure = pressure
    m = 0.8
    al = 1E-5
  [../]
  [./ppss_nodal]
    type = PorousFlow1PhaseP_VG # irrelevant in this fully-saturated situation
    at_nodes = true
    porepressure = pressure
    m = 0.8
    al = 1E-5
  [../]
  [./massfrac]
    type = PorousFlowMassFraction
    at_nodes = true
  [../]
  [./dens0]
    type = PorousFlowIdealGas
    at_nodes = true
    molar_mass = 1.4
    phase = 0
  [../]
  [./dens_all]
    type = PorousFlowJoiner
    at_nodes = true
    include_old = true
    material_property = PorousFlow_fluid_phase_density_nodal
  [../]
  [./dens0_qp]
    type = PorousFlowIdealGas
    molar_mass = 1.4
    phase = 0
  [../]
  [./dens_all_at_quadpoints]
    type = PorousFlowJoiner
    material_property = PorousFlow_fluid_phase_density_qp
    at_nodes = false
  [../]
  [./permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1.1 0 0 0 1.1 0 0 0 1.1'
  [../]
  [./relperm]
    type = PorousFlowRelativePermeabilityCorey # irrelevant in this fully-saturated situation
    at_nodes = true
    n = 2
    phase = 0
  [../]
  [./relperm_all]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_relative_permeability_nodal
  [../]
  [./visc0]
    type = PorousFlowViscosityConst
    at_nodes = true
    viscosity = 1.2
    phase = 0
  [../]
  [./visc_all]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_viscosity_nodal
  [../]
  [./enthalpy0]
    type = PorousFlowEnthalpy
    at_nodes = true
    phase = 0
  [../]
  [./enthalpy_all]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_fluid_phase_enthalpy_nodal
  [../]
  [./fluid_energy0]
    type = PorousFlowInternalEnergyIdeal
    at_nodes = true
    specific_heat_capacity = 1.3
    phase = 0
  [../]
  [./energy_all]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_fluid_phase_internal_energy_nodal
  [../]
[]

[BCs]
  [./leftp]
    type = DirichletBC
    variable = pressure
    boundary = left
    value = 200
  [../]
  [./leftt]
    type = DirichletBC
    variable = temp
    boundary = left
    value = 180
  [../]
  [./newtonp]
    type = PorousFlowPiecewiseLinearSink
    variable = pressure
    boundary = right
    pt_vals = '-200 0 200'
    multipliers = '-200 0 200'
    use_mobility = true
    use_relperm = true
    fluid_phase = 0
    flux_function = 0.005 # 1/2/L
  [../]
  [./newtont]
    type = PorousFlowPiecewiseLinearSink
    variable = temp
    boundary = right
    pt_vals = '-200 0 200'
    multipliers = '-200 0 200'
    use_mobility = true
    use_relperm = true
    use_enthalpy = true
    fluid_phase = 0
    flux_function = 0.005 # 1/2/L
  [../]
[]

[VectorPostprocessors]
  [./porepressure]
    type = LineValueSampler
    variable = pressure
    start_point = '0 0.5 0'
    end_point = '100 0.5 0'
    sort_by = x
    num_points = 11
    execute_on = timestep_end
  [../]
  [./temperature]
    type = LineValueSampler
    variable = temp
    start_point = '0 0.5 0'
    end_point = '100 0.5 0'
    sort_by = x
    num_points = 11
    execute_on = timestep_end
  [../]
[]

[Preconditioning]
  [./andy]
    type = SMP
    full = true
    petsc_options = '-snes_converged_reason'
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -snes_max_it -sub_pc_factor_shift_type -pc_asm_overlap -snes_atol -snes_rtol'
    petsc_options_value = 'gmres asm lu 100 NONZERO 2 1E-15 1E-10'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = Newton
[]

[Outputs]
  file_base = nc08
  execute_on = timestep_end
  exodus = true
  [./along_line]
    type = CSV
    execute_vector_postprocessors_on = timestep_end
  [../]
[]
