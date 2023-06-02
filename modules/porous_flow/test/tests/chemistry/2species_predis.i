# PorousFlow analogy of chemical_reactions/test/tests/solid_kinetics/2species_without_action.i
#
# Simple equilibrium reaction example to illustrate the use of PorousFlowAqueousPreDisChemistry
#
# In this example, two primary species a and b diffuse towards each other from
# opposite ends of a porous medium, reacting when they meet to form a mineral
# precipitate. The kinetic reaction is
#
# a + b = mineral
#
# where a and b are the primary species (reactants), and mineral is the precipitate.
# At the time of writing, the results of this test differ from chemical_reactions because
# in PorousFlow the mineral_concentration is measured in m^3 (precipitate) / m^3 (porous_material)
# in chemical_reactions the mineral_concentration is measured in m^3 (precipitate) / m^3 (fluid)
# ie, PorousFlow_mineral_concentration = porosity * chemical_reactions_mineral_concentration

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmax = 1
  ymax = 1
  nx = 40
[]

[Variables]
  [a]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0
  []
  [b]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0
  []
[]

[AuxVariables]
  [eqm_k]
    initial_condition = 1E-6
  []
  [pressure]
  []
  [mineral]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [mineral]
    type = PorousFlowPropertyAux
    property = mineral_concentration
    mineral_species = 0
    variable = mineral
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[Kernels]
  [mass_a]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = a
  []
  [diff_a]
    type = PorousFlowDispersiveFlux
    variable = a
    fluid_component = 0
    disp_trans = 0
    disp_long = 0
  []
  [predis_a]
    type = PorousFlowPreDis
    variable = a
    mineral_density = 1000
    stoichiometry = 1
  []
  [mass_b]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = b
  []
  [diff_b]
    type = PorousFlowDispersiveFlux
    variable = b
    fluid_component = 1
    disp_trans = 0
    disp_long = 0
  []
  [predis_b]
    type = PorousFlowPreDis
    variable = b
    mineral_density = 1000
    stoichiometry = 1
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'a b'
    number_fluid_phases = 1
    number_fluid_components = 3
    number_aqueous_kinetic = 1
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 2e9 # huge, so mimic chemical_reactions
    density0 = 1000
    thermal_expansion = 0
    viscosity = 1e-3
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = 298.15
  []
  [ppss]
    type = PorousFlow1PhaseFullySaturated
    porepressure = pressure
  []
  [massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = 'a b'
  []
  [chem]
    type = PorousFlowAqueousPreDisChemistry
    primary_concentrations = 'a b'
    num_reactions = 1
    equilibrium_constants = eqm_k
    primary_activity_coefficients = '1 1'
    reactions = '1 1'
    specific_reactive_surface_area = '1.0'
    kinetic_rate_constant = '1.0e-8'
    activation_energy = '1.5e4'
    molar_volume = 1
    gas_constant = 8.314
    reference_temperature = 298.15
  []
  [mineral_conc]
    type = PorousFlowAqueousPreDisMineral
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.4
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    # porous_flow permeability / porous_flow viscosity = chemical_reactions conductivity = 4E-3
    permeability = '4E-6 0 0 0 4E-6 0 0 0 4E-6'
  []
  [relp]
    type = PorousFlowRelativePermeabilityConst
    phase = 0
  []
  [diff]
    type = PorousFlowDiffusivityConst
    # porous_flow diffusion_coeff * tortuousity * porosity = chemical_reactions diffusivity = 5E-4
    diffusion_coeff = '12.5E-4 12.5E-4 12.5E-4'
    tortuosity = 1.0
  []
[]

[BCs]
  [a_left]
    type = DirichletBC
    variable = a
    boundary = left
    value = 1.0e-2
  []
  [a_right]
    type = DirichletBC
    variable = a
    boundary = right
    value = 0
  []
  [b_left]
    type = DirichletBC
    variable = b
    boundary = left
    value = 0
  []
  [b_right]
    type = DirichletBC
    variable = b
    boundary = right
    value = 1.0e-2
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 5
  end_time = 50
[]

[Outputs]
  print_linear_residuals = true
  exodus = true
  perf_graph = true
  hide = eqm_k
[]
