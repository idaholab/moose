# PorousFlowPreDis, which is essentially checking the derivatives of the secondary concentrations in PorousFlowMassFractionAqueousPreDisChemistry
# Dissolution with temperature, with two primary variables = 0
[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [a]
    initial_condition = 0.0
  []
  [b]
    initial_condition = 0.0
  []
  [temp]
    initial_condition = 0.5
  []
[]

[AuxVariables]
  [eqm_k]
    initial_condition = 1.234
  []
  [ini_sec_conc]
    initial_condition = 0.222
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Kernels]
  [a]
    type = PorousFlowPreDis
    variable = a
    mineral_density = 1E5
    stoichiometry = 2
  []
  [b]
    type = PorousFlowPreDis
    variable = b
    mineral_density = 2.2E5
    stoichiometry = 3
  []
  [temp]
    type = Diffusion
    variable = temp
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'a b temp'
    number_fluid_phases = 1
    number_fluid_components = 3
    number_aqueous_kinetic = 1
  []
[]

[AuxVariables]
  [pressure]
  []
[]

[Materials]
  [porosity]
    type = PorousFlowPorosity
    porosity_zero = 0.9
  []
  [temperature]
    type = PorousFlowTemperature
    temperature = temp
  []
  [ppss]
    type = PorousFlow1PhaseFullySaturated
    porepressure = pressure
  []
  [massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = 'a b'
  []
  [predis]
    type = PorousFlowAqueousPreDisChemistry
    primary_concentrations = 'a b'
    num_reactions = 1
    equilibrium_constants = eqm_k
    primary_activity_coefficients = '0.5 0.8'
    reactions = '1 3'
    specific_reactive_surface_area = -44.4E-2
    kinetic_rate_constant = 0.678
    activation_energy = 4.4
    molar_volume = 3.3
    reference_temperature = 1
    gas_constant = 7.4
    theta_exponent = 1.0
    eta_exponent = 1.2
  []
  [mineral]
    type = PorousFlowAqueousPreDisMineral
    initial_concentrations = ini_sec_conc
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
    petsc_options = '-snes_test_display'
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -snes_type'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000 test'
  []
[]
