# Checking that the mass postprocessor correctly calculates the mass
# of each component in each phase, as well as the total mass of each
# component in all phases. Also tests that optional saturation threshold
# gives the correct mass
# 2phase, 2component, constant porosity
# saturation_threshold set to 0.6 for phase 1

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmin = 0
  xmax = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [./pp]
  [../]
  [./sat]
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

[ICs]
  [./pinit]
    type = ConstantIC
    value = 1
    variable = pp
  [../]
  [./satinit]
    type = FunctionIC
    function = 1-x
    variable = sat
  [../]
[]

[Kernels]
  [./mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  [../]
  [./mass1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = sat
  [../]
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp sat'
    number_fluid_phases = 2
    number_fluid_components = 2
  [../]
[]

[Materials]
  [./temperature]
    type = PorousFlowTemperature
    at_nodes = true
  [../]
  [./ppss]
    type = PorousFlow2PhasePS
    at_nodes = true
    phase0_porepressure = pp
    phase1_saturation = sat
  [../]
  [./massfrac]
    type = PorousFlowMassFraction
    at_nodes = true
    mass_fraction_vars = 'massfrac_ph0_sp0 massfrac_ph1_sp0'
  [../]
  [./dens0]
    type = PorousFlowDensityConstBulk
    at_nodes = true
    density_P0 = 1
    bulk_modulus = 1
    phase = 0
  [../]
  [./dens1]
    type = PorousFlowDensityConstBulk
    at_nodes = true
    density_P0 = 0.1
    bulk_modulus = 1
    phase = 1
  [../]
  [./dens_all]
    type = PorousFlowJoiner
    include_old = true
    at_nodes = true
    material_property = PorousFlow_fluid_phase_density_nodal
  [../]
  [./porosity]
    type = PorousFlowPorosityConst
    at_nodes = true
    porosity = 0.1
  [../]
[]

[Postprocessors]
  [./comp0_phase0_mass]
    type = PorousFlowFluidMass
    fluid_component = 0
    phase = 0
  [../]
  [./comp0_phase1_mass]
    type = PorousFlowFluidMass
    fluid_component = 0
    phase = 1
  [../]
  [./comp0_total_mass]
    type = PorousFlowFluidMass
    fluid_component = 0
  [../]
  [./comp1_phase0_mass]
    type = PorousFlowFluidMass
    fluid_component = 1
    phase = 0
  [../]
  [./comp1_phase1_mass]
    type = PorousFlowFluidMass
    fluid_component = 1
    phase = 1
  [../]
  [./comp1_total_mass]
    type = PorousFlowFluidMass
    fluid_component = 1
  [../]
  [./comp1_phase1_threshold_mass]
    type = PorousFlowFluidMass
    fluid_component = 1
    phase = 1
    saturation_threshold = 0.6
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  end_time = 1
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = mass06
  csv = true
[]
