# Check derivatives of mass-fraction, but using Equilibrium chemistry
[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [a]
    initial_condition = 0.1
  []
  [b]
    initial_condition = 0.2
  []
  [h2o_dummy]
  []
[]

[AuxVariables]
  [eqm_k0]
    initial_condition = 1.234E-4
  []
  [eqm_k1]
    initial_condition = 0.987E-4
  []
  [eqm_k2]
    initial_condition = 0.5E-4
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
    type = PorousFlowMassTimeDerivative
    variable = a
    fluid_component = 0
  []
  [b]
    type = PorousFlowMassTimeDerivative
    variable = b
    fluid_component = 1
  []
  [h2o_dummy]
    # note that in real simulations this Kernel would not be used
    # It is just here to check derivatives
    type = PorousFlowMassTimeDerivative
    variable = h2o_dummy
    fluid_component = 2
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'a b'
    number_fluid_phases = 1
    number_fluid_components = 3
    number_aqueous_equilibrium = 3
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
  []
  [ppss]
    type = PorousFlow1PhaseFullySaturated
    porepressure = pressure
  []
  [massfrac]
    type = PorousFlowMassFractionAqueousEquilibriumChemistry
    mass_fraction_vars = 'a b'
    num_reactions = 3
    equilibrium_constants = 'eqm_k0 eqm_k1 eqm_k2'
    primary_activity_coefficients = '1 1.2'
    secondary_activity_coefficients = '1 2 3'
    reactions = '1 2
                 2.2 -1
                 -2 1'
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [porosity]
    type = PorousFlowPorosity
    porosity_zero = 0.1
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
