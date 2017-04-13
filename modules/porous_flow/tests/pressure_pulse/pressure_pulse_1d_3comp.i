# Pressure pulse in 1D with 1 phase but 3 components (viscosity, relperm, etc are independent of mass-fractions) - transient
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
  [./pp]
    initial_condition = 2E6
  [../]
  [./massfrac0]
    initial_condition = 0.1
  [../]
  [./massfrac1]
    initial_condition = 0.3
  [../]
[]

[Kernels]
  [./mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  [../]
  [./flux0]
    type = PorousFlowAdvectiveFlux
    variable = pp
    gravity = '0 0 0'
    fluid_component = 0
  [../]
  [./mass1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = massfrac0
  [../]
  [./flux1]
    type = PorousFlowAdvectiveFlux
    variable = massfrac0
    gravity = '0 0 0'
    fluid_component = 1
  [../]
  [./mass2]
    type = PorousFlowMassTimeDerivative
    fluid_component = 2
    variable = massfrac1
  [../]
  [./flux2]
    type = PorousFlowAdvectiveFlux
    variable = massfrac1
    gravity = '0 0 0'
    fluid_component = 2
  [../]
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp massfrac0 massfrac1'
    number_fluid_phases = 1
    number_fluid_components = 3
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
  [./ppss]
    type = PorousFlow1PhaseP_VG
    porepressure = pp
    al = 1E-7
    m = 0.5
  [../]
  [./ppss_nodal]
    type = PorousFlow1PhaseP_VG
    at_nodes = true
    porepressure = pp
    al = 1E-7
    m = 0.5
  [../]
  [./massfrac]
    type = PorousFlowMassFraction
    at_nodes = true
    mass_fraction_vars = 'massfrac0 massfrac1'
  [../]
  [./dens0]
    type = PorousFlowDensityConstBulk
    at_nodes = true
    density_P0 = 1000
    bulk_modulus = 2E9
    phase = 0
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
  [./dens_all_at_quadpoints]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_fluid_phase_density_qp
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
  [./relperm]
    type = PorousFlowRelativePermeabilityCorey
    at_nodes = true
    n = 0
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
    viscosity = 1E-3
    phase = 0
  [../]
  [./visc_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_viscosity_nodal
  [../]
[]

[BCs]
  [./left]
    type = PresetBC
    boundary = left
    value = 3E6
    variable = pp
  [../]
[]

[Preconditioning]
  [./andy]
    type = SMP
    full = true
    petsc_options = '-snes_converged_reason -ksp_diagonal_scale -ksp_diagonal_scale_fix -ksp_gmres_modifiedgramschmidt -snes_linesearch_monitor'
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap -snes_atol -snes_rtol -snes_max_it -ksp_rtol -ksp_atol'
    petsc_options_value = 'gmres      asm      lu           NONZERO                   2               1E-7 1E-10 20 1E-10 1E-100'
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
    variable = pp
    point = '0 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p010]
    type = PointValue
    variable = pp
    point = '10 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p020]
    type = PointValue
    variable = pp
    point = '20 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p030]
    type = PointValue
    variable = pp
    point = '30 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p040]
    type = PointValue
    variable = pp
    point = '40 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p050]
    type = PointValue
    variable = pp
    point = '50 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p060]
    type = PointValue
    variable = pp
    point = '60 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p070]
    type = PointValue
    variable = pp
    point = '70 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p080]
    type = PointValue
    variable = pp
    point = '80 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p090]
    type = PointValue
    variable = pp
    point = '90 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p100]
    type = PointValue
    variable = pp
    point = '100 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./mf_0_010]
    type = PointValue
    variable = massfrac0
    point = '10 0 0'
    execute_on = 'timestep_end'
  [../]
  [./mf_1_010]
    type = PointValue
    variable = massfrac1
    point = '10 0 0'
    execute_on = 'timestep_end'
  [../]
[]

[Outputs]
  file_base = pressure_pulse_1d_3comp
  print_linear_residuals = true
  exodus = true
  csv = true
[]
