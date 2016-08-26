# Example of accessing properties using the PorousFlowPropertyAux AuxKernel for
# each phase and fluid component (as required).

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[Variables]
  [./pwater]
    initial_condition = 1e6
  [../]
  [./sgas]
    initial_condition = 0.3
  [../]
[]

[AuxVariables]
  [./pressure_gas]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./saturation_water]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./density_water]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./density_gas]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./viscosity_water]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./viscosity_gas]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./x0_water]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.1
  [../]
  [./x0_gas]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.8
  [../]
  [./x1_water]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./x1_gas]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./relperm_water]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./relperm_gas]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./pressure_gas]
    type = PorousFlowPropertyAux
    variable = pressure_gas
    property = pressure
    phase = 1
    execute_on = timestep_end
  [../]
  [./saturation_water]
    type = PorousFlowPropertyAux
    variable = saturation_water
    property = saturation
    phase = 0
    execute_on = timestep_end
  [../]
  [./density_water]
    type = PorousFlowPropertyAux
    variable = density_water
    property = density
    phase = 0
    execute_on = timestep_end
  [../]
  [./density_gas]
    type = PorousFlowPropertyAux
    variable = density_gas
    property = density
    phase = 1
    execute_on = timestep_end
  [../]
  [./viscosity_water]
    type = PorousFlowPropertyAux
    variable = viscosity_water
    property = viscosity
    phase = 0
    execute_on = timestep_end
  [../]
  [./viscosity_gas]
    type = PorousFlowPropertyAux
    variable = viscosity_gas
    property = viscosity
    phase = 1
    execute_on = timestep_end
  [../]
  [./relperm_water]
    type = PorousFlowPropertyAux
    variable = relperm_water
    property = relperm
    phase = 0
    execute_on = timestep_end
  [../]
  [./relperm_gas]
    type = PorousFlowPropertyAux
    variable = relperm_gas
    property = relperm
    phase = 1
    execute_on = timestep_end
  [../]
  [./x1water]
    type = PorousFlowPropertyAux
    variable = x1_water
    property = mass_fraction
    phase = 0
    fluid_component = 1
    execute_on = timestep_end
  [../]
  [./x1gas]
    type = PorousFlowPropertyAux
    variable = x1_gas
    property = mass_fraction
    phase = 1
    fluid_component = 1
    execute_on = timestep_end
  [../]
[]

[Kernels]
  [./mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pwater
  [../]
  [./flux0]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = pwater
  [../]
  [./mass1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = sgas
  [../]
  [./flux1]
    type = PorousFlowAdvectiveFlux
    fluid_component = 1
    variable = sgas
  [../]
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pwater sgas'
    number_fluid_phases = 2
    number_fluid_components = 2
  [../]
[]

[Materials]
  [./temperature]
    type = PorousFlowTemperature
  [../]
  [./nnn]
    type = PorousFlowNodeNumber
    on_initial_only = true
  [../]
  [./ppss]
    type = PorousFlow2PhasePS_VG
    phase0_porepressure = pwater
    phase1_saturation = sgas
    m = 0.5
    p0 = 1e5
    pc_max = 1e7
    sat_lr = 0.1
    sat_ls = 1
  [../]
  [./massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = 'x0_water x0_gas'
  [../]
  [./dens0]
    type = PorousFlowDensityConstBulk
    density_P0 = 1000
    bulk_modulus = 1e9
    phase = 0
  [../]
  [./dens1]
    type = PorousFlowDensityConstBulk
    density_P0 = 20
    bulk_modulus = 1e9
    phase = 1
  [../]
  [./dens_all]
    type = PorousFlowJoiner
    include_old = true
    material_property = PorousFlow_fluid_phase_density
  [../]
  [./dens_qp_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_fluid_phase_density_qp
    at_qps = true
  [../]
  [./visc0]
    type = PorousFlowViscosityConst
    viscosity = 1e-3
    phase = 0
  [../]
  [./visc1]
    type = PorousFlowViscosityConst
    viscosity = 1e-4
    phase = 1
  [../]
  [./visc_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_viscosity
  [../]
  [./permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1e-12 0 0 0 1e-12 0 0 0 1e-12'
  [../]
  [./relperm0]
    type = PorousFlowRelativePermeabilityCorey
    n_j = 2
    phase = 0
  [../]
  [./relperm1]
    type = PorousFlowRelativePermeabilityCorey
    n_j = 3
    phase = 1
  [../]
  [./relperm_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_relative_permeability
  [../]
  [./porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  end_time = 1
  nl_abs_tol = 1e-12
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Outputs]
  exodus = true
[]
