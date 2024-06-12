# PorousFlowPreDis, which is essentially checking the derivatives of the secondary concentrations in PorousFlowMassFractionAqueousPreDisChemistry
# Dissolution with temperature, with three primary variables and four reactions, and some zero concentrations
[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [a]
    initial_condition = 0
  []
  [b]
    initial_condition = 0
  []
  [c]
    initial_condition = 0
  []
  [temp]
    initial_condition = 0.5
  []
[]

[AuxVariables]
  [eqm_k0]
    initial_condition = 1.234
  []
  [eqm_k1]
    initial_condition = 1.999
  []
  [eqm_k2]
    initial_condition = 0.789
  []
  [eqm_k3]
    initial_condition = 1.111
  []
  [ini_sec_conc0]
    initial_condition = 0.02
  []
  [ini_sec_conc1]
    initial_condition = 0.04
  []
  [ini_sec_conc2]
    initial_condition = 0.06
  []
  [ini_sec_conc3]
    initial_condition = 0.08
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Kernels]
  [a]
    type = PorousFlowPreDis
    variable = a
    mineral_density = '1E10 2E10 3E10 4E10'
    stoichiometry = '1 1 2 0'
  []
  [b]
    type = PorousFlowPreDis
    variable = b
    mineral_density = '1.1E10 2.2E10 3.3E10 4.4E10'
    stoichiometry = '2 -2 0 0.5'
  []
  [c]
    type = PorousFlowPreDis
    variable = c
    mineral_density = '0.1E10 0.2E10 0.3E10 0.4E10'
    stoichiometry = '3 -3 0 1'
  []
  [temp]
    type = Diffusion
    variable = temp
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'a b c temp'
    number_fluid_phases = 1
    number_fluid_components = 4
    number_aqueous_kinetic = 4
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
    mass_fraction_vars = 'a b c'
  []
  [predis]
    type = PorousFlowAqueousPreDisChemistry
    primary_concentrations = 'a b c'
    num_reactions = 4
    equilibrium_constants = 'eqm_k0 eqm_k1 eqm_k2 eqm_k3'
    primary_activity_coefficients = '0.5 0.8 0.9'
    reactions = '0.5 2 3
                 1.5 -2 3
                 2 0 0
                 0 0.5 1'
    specific_reactive_surface_area = '-44.4E-2 22.1E-2 32.1E-1 -50E-2'
    kinetic_rate_constant = '0.678 0.999 1.23 0.3'
    activation_energy = '4.4 3.3 4.5 4.0'
    molar_volume = '3.3 4.4 5.5 6.6'
    reference_temperature = 1
    gas_constant = 7.4
    theta_exponent = '1.0 1.1 1.2 0.9'
    eta_exponent = '1.2 1.01 1.1 1.2'
  []
  [mineral]
    type = PorousFlowAqueousPreDisMineral
    initial_concentrations = 'ini_sec_conc0 ini_sec_conc1 ini_sec_conc2 ini_sec_conc3'
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
