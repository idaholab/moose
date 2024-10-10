[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [pp_CO2]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_condition = 1e6
  []
  [mass_frac_CH4]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_condition = 0.2
  []
[]

[FVKernels]
  [mass_CO2]
    type = FVDiffusion
    variable = pp_CO2
    coeff = 1
  []
  [mass_CH4]
    type = FVDiffusion
    variable = mass_frac_CH4
    coeff = 1
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp_CO2 mass_frac_CH4'
    number_fluid_phases = 1
    number_fluid_components = 2
  []
[]

[FluidProperties]
  [CO2]
    type = CO2FluidProperties
  []
  [CH4]
    type = MethaneFluidProperties
  []
[]

[Materials]
  [ps]
    type = ADPorousFlow1PhaseFullySaturated
    porepressure = pp_CO2
  []
  [porosity]
    type = ADPorousFlowPorosityConst
    porosity = 0.1
  []
  [CO2]
    type = ADPorousFlowMultiComponentGasMixture
    fp = 'CH4 CO2'
    mass_fraction = mass_frac_CH4
    phase = 0
  []
  [massfrac]
    type = ADPorousFlowMassFraction
    mass_fraction_vars = mass_frac_CH4
  []
  [temperature]
    type = ADPorousFlowTemperature
    temperature = 313
  []
[]

[Preconditioning]
  [basic]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -pc_factor_shift_type '
    petsc_options_value = 'lu           NONZERO'
  []
[]

[Executioner]
  type = Transient
  end_time = 1
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-12
[]

[Postprocessors]
  [density]
    type = ADElementIntegralMaterialProperty
    mat_prop = 'PorousFlow_fluid_phase_density_qp0'
    execute_on = 'initial timestep_end'
  []
  [viscosity]
    type = ADElementIntegralMaterialProperty
    mat_prop = 'PorousFlow_viscosity_qp0'
    execute_on = 'initial timestep_end'
  []
  [internal_energy]
    type = ADElementIntegralMaterialProperty
    mat_prop = 'PorousFlow_fluid_phase_internal_energy_qp0'
    execute_on = 'initial timestep_end'
  []
  [enthalpy]
    type = ADElementIntegralMaterialProperty
    mat_prop = 'PorousFlow_fluid_phase_enthalpy_qp0'
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  file_base = multicomponent_gas_mixture_out
  csv = true
[]
