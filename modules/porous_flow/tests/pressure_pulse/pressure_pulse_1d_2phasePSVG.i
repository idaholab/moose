# Pressure pulse in 1D with 2 phases, 2components - transient

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmin = 0
  xmax = 100
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[Variables]
  [./ppwater]
    initial_condition = 2e6
  [../]
  [./sgas]
    initial_condition = 0.3
  [../]
[]

[AuxVariables]
  [./massfrac_ph0_sp0]
    initial_condition = 1
  [../]
  [./massfrac_ph1_sp0]
    initial_condition = 0
  [../]
  [./ppgas]
    family = MONOMIAL
    order = FIRST
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
    fluid_component = 0
  [../]
  [./mass1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = sgas
  [../]
  [./flux1]
    type = PorousFlowAdvectiveFlux
    variable = sgas
    fluid_component = 1
  [../]
[]

[AuxKernels]
  [./ppgas]
    type = PorousFlowPropertyAux
    property = pressure
    phase = 1
    variable = ppgas
  [../]
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'ppwater sgas'
    number_fluid_phases = 2
    number_fluid_components = 2
  [../]
[]

[Materials]
  [./temperature]
    type = PorousFlowTemperature
  [../]
  [./temperature_nodal]
    type = PorousFlowTemperature
    at_nodes = true
  [../]
  [./ppss_qp]
    type = PorousFlow2PhasePS_VG
    phase0_porepressure = ppwater
    phase1_saturation = sgas
    m = 0.5
    sat_lr = 0.3
    p0 = 1e4
    pc_max = -1e6
  [../]
  [./ppss]
    type = PorousFlow2PhasePS_VG
    at_nodes = true
    phase0_porepressure = ppwater
    phase1_saturation = sgas
    m = 0.5
    sat_lr = 0.3
    p0 = 1e4
    pc_max = -1e6
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
  [./dens1]
    type = PorousFlowDensityConstBulk
    at_nodes = true
    density_P0 = 10
    bulk_modulus = 2E7
    phase = 1
  [../]
  [./dens_all]
    type = PorousFlowJoiner
    at_nodes = true
    include_old = true
    material_property = PorousFlow_fluid_phase_density_nodal
  [../]
  [./dens0_qp]
    type = PorousFlowDensityConstBulk
    density_P0 = 1000
    bulk_modulus = 2E9
    phase = 0
  [../]
  [./dens1_qp]
    type = PorousFlowDensityConstBulk
    density_P0 = 10
    bulk_modulus = 2E7
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
    permeability = '1e-15 0 0 0 1e-15 0 0 0 1e-15'
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
    viscosity = 1e-3
    phase = 0
  [../]
  [./visc1]
    type = PorousFlowViscosityConst
    at_nodes = true
    viscosity = 1e-5
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
    value = 3e6
    variable = ppwater
  [../]
  [./rightwater]
    type = DirichletBC
    boundary = right
    value = 2e6
    variable = ppwater
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-20 10000'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1e3
  end_time = 1e4
[]

[VectorPostprocessors]
  [./pp]
    type = LineValueSampler
    execute_on = timestep_end
    sort_by = x
    variable = 'ppwater ppgas'
    start_point = '0 0 0'
    end_point = '100 0 0'
    num_points = 11
  [../]
[]

[Outputs]
  file_base = pressure_pulse_1d_2phasePSVG
  print_linear_residuals = false
  csv = true
  execute_on = final
[]
