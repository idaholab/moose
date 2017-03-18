# Pressure pulse in 1D with 1 phase - transient
# using the PorousFlowFullySaturatedDarcyBase Kernel
# and the PorousFlowFullySaturatedMassTimeDerivative Kernel
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
  xmin = 0
  xmax = 100
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [./pp]
    initial_condition = 2E6
  [../]
[]

[Kernels]
  [./mass0]
    type = PorousFlowFullySaturatedMassTimeDerivative
    variable = pp
  [../]
  [./flux]
    type = PorousFlowFullySaturatedDarcyBase
    variable = pp
    gravity = '0 0 0'
  [../]
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp'
    number_fluid_phases = 1
    number_fluid_components = 1
  [../]
[]

[Materials]
  [./temperature_qp]
    type = PorousFlowTemperature
  [../]
  [./ppss_qp]
    type = PorousFlow1PhaseP
    porepressure = pp
  [../]
  [./massfrac]
    type = PorousFlowMassFraction
  [../]
  [./dens0_qp]
    type = PorousFlowDensityConstBulk
    density_P0 = 1000
    bulk_modulus = 2E9
    phase = 0
  [../]
  [./dens_all_at_quadpoints]
    type = PorousFlowJoiner
    material_property = PorousFlow_fluid_phase_density_qp
    at_nodes = false
  [../]
  [./porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  [../]
  [./biot_modulus]
    type = PorousFlowConstantBiotModulus
    fluid_bulk_modulus = 2E9
  [../]
  [./permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1E-15 0 0 0 1E-15 0 0 0 1E-15'
  [../]
  [./visc0]
    type = PorousFlowViscosityConst
    viscosity = 1E-3
    phase = 0
  [../]
  [./visc_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_viscosity_qp
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    boundary = left
    value = 3E6
    variable = pp
  [../]
[]

[Preconditioning]
  [./andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-20 10000'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1E3
  end_time = 1E4
[]

[Postprocessors]
  [./p005]
    type = PointValue
    variable = pp
    point = '5 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p015]
    type = PointValue
    variable = pp
    point = '15 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p025]
    type = PointValue
    variable = pp
    point = '25 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p035]
    type = PointValue
    variable = pp
    point = '35 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p045]
    type = PointValue
    variable = pp
    point = '45 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p055]
    type = PointValue
    variable = pp
    point = '55 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p065]
    type = PointValue
    variable = pp
    point = '65 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p075]
    type = PointValue
    variable = pp
    point = '75 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p085]
    type = PointValue
    variable = pp
    point = '85 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p095]
    type = PointValue
    variable = pp
    point = '95 0 0'
    execute_on = 'initial timestep_end'
  [../]
[]

[Outputs]
  file_base = pressure_pulse_1d_fully_saturated_2
  print_linear_residuals = false
  csv = true
[]
