# Exception test
# Chemistry-dependent porosity is not available with automatic differentiation
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
    initial_condition = 1E-6
  []
  [a]
    initial_condition = 0.5
  []
  [ini_mineral_conc]
    initial_condition = 0.2
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
  [temperature_qp]
    type = PorousFlowTemperature
    temperature = 1
  []
  [predis_qp]
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
  [mineral_conc_qp]
    type = PorousFlowAqueousPreDisMineral
    initial_concentrations = ini_mineral_conc
  []
  [porosity]
    # Everything here is valid for the non-AD PorousFlowPorosity: only the AD version is an error
    type = ADPorousFlowPorosity
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
