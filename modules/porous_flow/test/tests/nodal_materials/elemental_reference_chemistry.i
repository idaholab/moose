# PorousFlowPorosity at the nodes, with elemental reference mineral concentrations.
#
# Companion to elemental_reference.i, which covers reference_temperature.  Here the
# elemental variable is supplied to reference_chemistry and initial_mineral_concentrations
# instead, which PorousFlowPorosity reads through the same path.
#
# ini_mineral_conc is a CONSTANT MONOMIAL AuxVariable, so it holds one value per element
# rather than one per node and must be read at the quadpoints.  Reading it by degree of
# freedom, once per node, runs off the end of the one-entry array.  That read is silent in
# an optimised build, so this test is only meaningful under a devel or dbg build.
#
# Note the mass time derivative: it is mass-lumped, so it requests nodal Material
# properties and is what causes the porosity Material to be evaluated at the nodes at all.
# Without a lumped or upwinded kernel every Material is a quadpoint Material and this path
# is never taken.
#
# The chemistry is that of chemistry/precipitation_porosity_change.i: a <==> mineral, with
# a held fixed so the mineral grows at a constant rate and the porosity falls to match.

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 1
    nz = 1
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [porepressure]
    initial_condition = 1e6
  []
[]

[AuxVariables]
  [eqm_k]
    initial_condition = 0.5
  []
  [a]
    initial_condition = 0.5
  []
  [ini_mineral_conc]
    family = MONOMIAL
    order = CONSTANT # one degree of freedom per element, not one per node
    initial_condition = 0.2
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = porepressure
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'porepressure'
    number_fluid_phases = 1
    number_fluid_components = 2
    number_aqueous_kinetic = 1
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 2e9
    density0 = 1000
    thermal_expansion = 0
    viscosity = 1e-3
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = 1
    at_nodes = true
  []
  [ppss]
    type = PorousFlow1PhaseFullySaturated
    porepressure = porepressure
    at_nodes = true
  []
  [massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = a
    at_nodes = true
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
    at_nodes = true
  []
  [predis]
    type = PorousFlowAqueousPreDisChemistry
    primary_concentrations = a
    num_reactions = 1
    equilibrium_constants = eqm_k
    primary_activity_coefficients = 2
    reactions = 1
    specific_reactive_surface_area = 0.5
    kinetic_rate_constant = 0.6065306597126334
    activation_energy = 3
    molar_volume = 2
    gas_constant = 6
    reference_temperature = 0.5
    at_nodes = true
  []
  [mineral_conc]
    type = PorousFlowAqueousPreDisMineral
    initial_concentrations = ini_mineral_conc
    at_nodes = true
  []
  [porosity]
    type = PorousFlowPorosity
    chemical = true
    porosity_zero = 0.6
    reference_chemistry = ini_mineral_conc # the elemental AuxVariable
    initial_mineral_concentrations = ini_mineral_conc # and again here
    at_nodes = true
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
    solve_type = Newton
  []
[]

[Executioner]
  type = Transient
  nl_abs_tol = 1e-10
  dt = 0.1
  end_time = 0.4
[]

[Postprocessors]
  [fluid_mass]
    type = PorousFlowFluidMass
  []
  [pp]
    type = PointValue
    point = '0 0 0'
    variable = porepressure
  []
[]

[Outputs]
  csv = true
[]
