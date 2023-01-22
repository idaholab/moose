# Using a two-phase system (see precipitation.i for the single-phase)
# The saturation and porosity are chosen so that the results are identical to precipitation.i
#
# The precipitation reaction
#
# a <==> mineral
#
# produces "mineral".  Using mineral_density = fluid_density, theta = 1 = eta, the DE is
#
# a' = -(mineral / (porosity * saturation))' = rate * surf_area * molar_vol (1 - (1 / eqm_const) * (act_coeff * a)^stoi)
#
# The following parameters are used
#
# T_ref = 0.5 K
# T = 1 K
# activation_energy = 3 J/mol
# gas_constant = 6 J/(mol K)
# kinetic_rate_at_ref_T = 0.60653 mol/(m^2 s)
# These give rate = 0.60653 * exp(1/2) = 1 mol/(m^2 s)
#
# surf_area = 0.5 m^2/L
# molar_volume = 2 L/mol
# These give rate * surf_area * molar_vol = 1 s^-1
#
# equilibrium_constant = 0.5 (dimensionless)
# primary_activity_coefficient = 2 (dimensionless)
# stoichiometry = 1 (dimensionless)
# This means that 1 - (1 / eqm_const) * (act_coeff * a)^stoi = 1 - 4 a, which is negative for a > 0.25, ie precipitation for a(t=0) > 0.25
#
# The solution of the DE is
# a = eqm_const / act_coeff + (a(t=0) - eqm_const / act_coeff) exp(-rate * surf_area * molar_vol * act_coeff * t / eqm_const)
#   = 0.25 + (a(t=0) - 0.25) exp(-4 * t)
# c = c(t=0) - (a - a(t=0)) * (porosity * saturation)
#
# This test checks that (a + c / (porosity * saturation)) is time-independent, and that a follows the above solution
#
# Aside:
#    The exponential curve is not followed exactly because moose actually solves
#    (a - a_old)/dt = rate * surf_area * molar_vol (1 - (1 / eqm_const) * (act_coeff * a)^stoi)
#    which does not give an exponential exactly, except in the limit dt->0
[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [a]
    initial_condition = 0.9
  []
[]

[AuxVariables]
  [eqm_k]
    initial_condition = 0.5
  []
  [pressure0]
  []
  [saturation1]
    initial_condition = 0.25
  []
  [b]
    initial_condition = 0.123
  []
  [ini_mineral_conc]
    initial_condition = 0.2
  []
  [mineral]
    family = MONOMIAL
    order = CONSTANT
  []
  [should_be_static]
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
  [should_be_static]
    type = ParsedAux
    coupled_variables = 'mineral a'
    expression = 'a + mineral / 0.1'
    variable = should_be_static
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Kernels]
  [mass_a]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = a
  []
  [pre_dis]
    type = PorousFlowPreDis
    variable = a
    mineral_density = 1000
    stoichiometry = 1
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = a
    number_fluid_phases = 2
    number_fluid_components = 2
    number_aqueous_kinetic = 1
    aqueous_phase_number = 1
  []
  [pc]
    type = PorousFlowCapillaryPressureConst
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
    temperature = 1
  []
  [ppss]
    type = PorousFlow2PhasePS
    capillary_pressure = pc
    phase0_porepressure = pressure0
    phase1_saturation = saturation1
  []
  [mass_frac]
    type = PorousFlowMassFraction
    mass_fraction_vars = 'b a'
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
  []
  [mineral_conc]
    type = PorousFlowAqueousPreDisMineral
    initial_concentrations = ini_mineral_conc
  []
  [simple_fluid0]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [simple_fluid1]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 1
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.4
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
  nl_abs_tol = 1E-10
  dt = 0.01
  end_time = 1
[]

[Postprocessors]
  [a]
    type = PointValue
    point = '0 0 0'
    variable = a
  []
  [should_be_static]
    type = PointValue
    point = '0 0 0'
    variable = should_be_static
  []
[]
[Outputs]
  interval = 10
  csv = true
  perf_graph = true
[]
