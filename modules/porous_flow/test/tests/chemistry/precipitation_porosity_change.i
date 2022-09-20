# Test to illustrate porosity evolution due to precipitation
#
# The precipitation reaction
#
# a <==> mineral
#
# produces "mineral".  Using theta = 1 = eta, the DE that describes the prcipitation is
# reaction_rate = rate * surf_area * molar_vol (1 - (1 / eqm_const) * (act_coeff * a)^stoi)
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
# This means that 1 - (1 / eqm_const) * (act_coeff * a)^stoi = 1 - 4 a, which is negative (ie precipitation) for a > 0.25
#
# a is held fixed at 0.5, so
# reaction_rate = - (1 - 2) = 1
#
# The mineral volume fraction evolves according to
# Mineral = mineral_old + dt * porosity_old * reaction_rate = mineral_old + dt * porosity_old
#
# Porosity evolves according to
# porosity = porosity(t=0) - (mineral - mineral(t=0))
#          = porosity(t=0) - (mineral_old + dt * porosity_old * reaction_rate - mineral(t=0))
#
# Specifically:
# time mineral porosity
# 0    0.2     0.6
# 0.1  0.26    0.54
# 0.2  0.314   0.486
# 0.3  0.3626  0.4374
# 0.4  0.40634 0.39366
[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [dummy]
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
    initial_condition = 0.2
  []
  [mineral]
    family = MONOMIAL
    order = CONSTANT
  []
  [porosity]
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
  [porosity]
    type = PorousFlowPropertyAux
    property = porosity
    variable = porosity
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Kernels]
  [dummy]
    type = Diffusion
    variable = dummy
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = dummy
    number_fluid_phases = 1
    number_fluid_components = 2
    number_aqueous_kinetic = 1
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = 1
  []
  [ppss]
    type = PorousFlow1PhaseFullySaturated
    porepressure = dummy
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
  [porosity]
    type = PorousFlowPorosity
    chemical = true
    porosity_zero = 0.6
    reference_chemistry = ini_mineral_conc
    initial_mineral_concentrations = ini_mineral_conc
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
  dt = 0.1
  end_time = 0.4
[]

[Postprocessors]
  [porosity]
    type = PointValue
    point = '0 0 0'
    variable = porosity
  []
  [c]
    type = PointValue
    point = '0 0 0'
    variable = mineral
  []
[]
[Outputs]
  csv = true
  perf_graph = true
[]
