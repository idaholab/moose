# Check derivatives of PorousFlowPorosity with chemical=true
[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [a]
    initial_condition = 0.1
  []
[]

[AuxVariables]
  [eqm_k0]
    initial_condition = 1.234
  []
  [eqm_k1]
    initial_condition = 0.987
  []
  [temp]
    initial_condition = 0.5
  []
  [ini_sec_conc0]
    initial_condition = 0.111
  []
  [ini_sec_conc1]
    initial_condition = 0.222
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Kernels]
  [a]
    type = PorousFlowMassTimeDerivative # this is rather irrelevant: we just want something with Porosity in it
    variable = a
    fluid_component = 0
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = a
    number_fluid_phases = 1
    number_fluid_components = 2
    number_aqueous_kinetic = 2
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
  []
[]

[AuxVariables]
  [pressure]
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = temp
    at_nodes = true
  []
  [ppss]
    type = PorousFlow1PhaseFullySaturated
    at_nodes = true
    porepressure = pressure
  []
  [massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = a
    at_nodes = true
  []
  [predis]
    type = PorousFlowAqueousPreDisChemistry
    primary_concentrations = a
    num_reactions = 2
    equilibrium_constants = 'eqm_k0 eqm_k1'
    primary_activity_coefficients = 1
    reactions = '1E-10
                 2E-10'  # so that mass_frac = a
    specific_reactive_surface_area = '-44.4E-2 -12E-2'
    kinetic_rate_constant = '0.678 0.7'
    activation_energy = '4.4 3.3'
    molar_volume = '3.3 2.2'
    reference_temperature = 1
    gas_constant = 7.4
    at_nodes = true
  []
  [mineral]
    type = PorousFlowAqueousPreDisMineral
    initial_concentrations = 'ini_sec_conc0 ini_sec_conc1'
    at_nodes = true
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    at_nodes = true
    phase = 0
  []
  [porosity]
    type = PorousFlowPorosity
    chemical = true
    porosity_zero = 0.1
    reference_chemistry = 'ini_sec_conc0 ini_sec_conc1'
    initial_mineral_concentrations = 'ini_sec_conc0 ini_sec_conc1'
    chemical_weights = '1.111 0.888' # so derivatives of porosity are big
    at_nodes = true
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 0.1
  end_time = 0.1
[]

[Preconditioning]
  [check]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -snes_type'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000 test'
  []
[]
