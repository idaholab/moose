# Exception test.
# Incorrect number of secondary activity coefficients

[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [a]
  []
  [b]
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Kernels]
  [a]
    type = Diffusion
    variable = a
  []
  [b]
    type = Diffusion
    variable = b
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'a b'
    number_fluid_phases = 1
    number_fluid_components = 3
    number_aqueous_equilibrium = 2
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
  []
[]

[AuxVariables]
  [eqm_k0]
    initial_condition = 1E2
  []
  [eqm_k1]
    initial_condition = 1E-2
  []
  [pressure]
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
  []
  [ppss]
    type = PorousFlow1PhaseFullySaturated
    porepressure = pressure
  []
  [massfrac]
    type = PorousFlowMassFractionAqueousEquilibriumChemistry
    mass_fraction_vars = 'a b'
    num_reactions = 2
    equilibrium_constants = 'eqm_k0 eqm_k1'
    primary_activity_coefficients = '1 1'
    secondary_activity_coefficients = '1'
    reactions = '2 0
                 1 1'
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  end_time = 1
[]
