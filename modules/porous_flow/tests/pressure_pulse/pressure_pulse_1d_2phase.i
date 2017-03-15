# Pressure pulse in 1D with 2 phases (with one having zero saturation), 2components - transient
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmin = 0
  xmax = 100
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [./ppwater]
    initial_condition = 2E6
  [../]
  [./ppgas]
    initial_condition = 2E6
  [../]
[]

[AuxVariables]
  [./massfrac_ph0_sp0]
    initial_condition = 1
  [../]
  [./massfrac_ph1_sp0]
    initial_condition = 0
  [../]
[]


[Kernels]
  [./mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = ppwater
  [../]
  [./flux0]
    type = PorousFlowAdvectiveFlux
    variable = ppwater
    gravity = '0 0 0'
    fluid_component = 0
  [../]
  [./mass1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = ppgas
  [../]
  [./flux1]
    type = PorousFlowAdvectiveFlux
    variable = ppgas
    gravity = '0 0 0'
    fluid_component = 1
  [../]
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'ppwater ppgas'
    number_fluid_phases = 2
    number_fluid_components = 2
  [../]
[]

[Materials]
  [./temperature]
    type = PorousFlowTemperature
    at_nodes = true
  [../]
  [./temperature_qp]
    type = PorousFlowTemperature
  [../]
  [./ppss]
    type = PorousFlow2PhasePP_VG
    at_nodes = true
    phase0_porepressure = ppwater
    phase1_porepressure = ppgas
    m = 0.5
    al = 1
  [../]
  [./ppss_qp]
    type = PorousFlow2PhasePP_VG
    phase0_porepressure = ppwater
    phase1_porepressure = ppgas
    m = 0.5
    al = 1
  [../]
  [./massfrac]
    type = PorousFlowMassFraction
    at_nodes = true
    mass_fraction_vars = 'massfrac_ph0_sp0 massfrac_ph1_sp0'
  [../]
  [./dens0]
    type = PorousFlowDensityConstBulk
    at_nodes = true
    density_P0 = 1000
    bulk_modulus = 2E9
    phase = 0
  [../]
  [./dens1] # this is irrelevant - there is no gas
    type = PorousFlowDensityConstBulk
    at_nodes = true
    density_P0 = 1
    bulk_modulus = 2E6
    phase = 1
  [../]
  [./dens_all]
    type = PorousFlowJoiner
    include_old = true
    at_nodes = true
    material_property = PorousFlow_fluid_phase_density_nodal
  [../]
  [./dens0_qp]
    type = PorousFlowDensityConstBulk
    density_P0 = 1000
    bulk_modulus = 2E9
    phase = 0
  [../]
  [./dens1_qp] # this is irrelevant - there is no gas
    type = PorousFlowDensityConstBulk
    density_P0 = 1
    bulk_modulus = 2E6
    phase = 1
  [../]
  [./dens_all_at_quadpoints]
    type = PorousFlowJoiner
    material_property = PorousFlow_fluid_phase_density_qp
    at_nodes = false
  [../]
  [./porosity]
    type = PorousFlowPorosityConst
    at_nodes = true
    porosity = 0.1
  [../]
  [./permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1E-15 0 0 0 1E-15 0 0 0 1E-15'
  [../]
  [./relperm_water]
    type = PorousFlowRelativePermeabilityCorey
    at_nodes = true
    n = 1
    phase = 0
  [../]
  [./relperm_gas]
    type = PorousFlowRelativePermeabilityCorey
    at_nodes = true
    n = 1
    phase = 1
  [../]
  [./relperm_all]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_relative_permeability_nodal
  [../]
  [./visc0]
    type = PorousFlowViscosityConst
    at_nodes = true
    viscosity = 1E-3
    phase = 0
  [../]
  [./visc1] # this is irrelevant - there is no gas
    type = PorousFlowViscosityConst
    at_nodes = true
    viscosity = 1E-5
    phase = 1
  [../]
  [./visc_all]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_viscosity_nodal
  [../]
[]

[BCs]
  [./leftwater]
    type = DirichletBC
    boundary = left
    value = 3E6
    variable = ppwater
  [../]
  [./leftgas]
    type = DirichletBC
    boundary = left
    value = 3E6
    variable = ppgas
  [../]
[]

[Preconditioning]
  [./andy]
    type = SMP
    full = true
    petsc_options = '-snes_converged_reason -ksp_diagonal_scale -ksp_diagonal_scale_fix -ksp_gmres_modifiedgramschmidt -snes_linesearch_monitor'
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'gmres      asm      lu           NONZERO                   2               1E-15       1E-20 20'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1E3
  end_time = 1E4
[]

[Postprocessors]
  [./p000]
    type = PointValue
    variable = ppwater
    point = '0 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p010]
    type = PointValue
    variable = ppwater
    point = '10 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p020]
    type = PointValue
    variable = ppwater
    point = '20 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p030]
    type = PointValue
    variable = ppwater
    point = '30 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p040]
    type = PointValue
    variable = ppwater
    point = '40 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p050]
    type = PointValue
    variable = ppwater
    point = '50 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p060]
    type = PointValue
    variable = ppwater
    point = '60 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p070]
    type = PointValue
    variable = ppwater
    point = '70 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p080]
    type = PointValue
    variable = ppwater
    point = '80 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p090]
    type = PointValue
    variable = ppwater
    point = '90 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p100]
    type = PointValue
    variable = ppwater
    point = '100 0 0'
    execute_on = 'initial timestep_end'
  [../]
[]

[Outputs]
  file_base = pressure_pulse_1d_2phase
  print_linear_residuals = false
  csv = true
[]
