# Exception test
# Incorrect number of stoichiometry
[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [./a]
  [../]
  [./b]
  [../]
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Kernels]
  [./a]
    type = PorousFlowPreDis
    variable = a
    mineral_density = 1
    stoichiometry = '2 3'
  [../]
  [./b]
    type = PorousFlowPreDis
    variable = b
    mineral_density = 1
    stoichiometry = 3
  [../]
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'a b'
    number_fluid_phases = 1
    number_fluid_components = 3
    number_aqueous_kinetic = 1
  [../]
[]

[Modules]
  [./FluidProperties]
    [./simple_fluid]
      type = SimpleFluidProperties
    [../]
  [../]
[]

[AuxVariables]
  [./pressure]
  [../]
[]

[Materials]
  [./temperature]
    type = PorousFlowTemperature
    at_nodes = true
  [../]
  [./ppss]
    type = PorousFlow1PhaseFullySaturated
    porepressure = pressure
    at_nodes = true
  [../]
  [./massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = 'a b'
    at_nodes = true
  [../]
  [./predis]
    type = PorousFlowAqueousPreDisChemistry
    primary_concentrations = 'a b'
    num_reactions = 1
    equilibrium_constants = '1E-6'
    primary_activity_coefficients = '1 1'
    reactions = '2 3'
    specific_reactive_surface_area = 1.0
    kinetic_rate_constant = 1.0e-8
    activation_energy = 1.5e4
    molar_volume = 1
    at_nodes = true
  [../]
  [./mineral]
    type = PorousFlowAqueousPreDisMineral
    at_nodes = true
  [../]
  [./porosity]
    type = PorousFlowPorosity
    porosity_zero = 0.1
    at_nodes = true
  [../]
  [./simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
    at_nodes = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  end_time = 1
[]
