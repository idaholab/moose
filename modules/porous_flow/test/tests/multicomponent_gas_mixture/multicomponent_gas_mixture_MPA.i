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
    initial_condition = 1
  []
  [mass_frac_CH4]
    initial_condition = 0.2
  []
[]

[Kernels]
  [mass_CO2]
    type = Diffusion
    variable = pp_CO2
  []
  [mass_CH4]
    type = Diffusion
    variable = mass_frac_CH4
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
    type = PorousFlow1PhaseFullySaturated
    porepressure = pp_CO2
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  []
  [CO2]
    type = PorousFlowMultiComponentGasMixture
    fp = 'CH4 CO2'
    mass_fraction = mass_frac_CH4
    phase = 0
    pressure_unit = MPA
  []
  [massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = mass_frac_CH4
  []
  [temperature]
    type = PorousFlowTemperature
    temperature = 313
  []
[]

[Preconditioning]
  [basic]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -pc_factor_shift_type '
    petsc_options_value = ' lu       NONZERO'
  []
[]

[Executioner]
  type = Transient
  end_time = 1
  nl_abs_tol = 1e-12
[]

[Postprocessors]
  [density]
    type = ElementIntegralMaterialProperty
    mat_prop = 'PorousFlow_fluid_phase_density_qp0'
    execute_on = 'initial timestep_end'
  []
  [viscosity]
    type = ElementIntegralMaterialProperty
    mat_prop = 'PorousFlow_viscosity_qp0'
    execute_on = 'initial timestep_end'
  []
  [internal_energy]
    type = ElementIntegralMaterialProperty
    mat_prop = 'PorousFlow_fluid_phase_internal_energy_qp0'
    execute_on = 'initial timestep_end'
  []
  [enthalpy]
    type = ElementIntegralMaterialProperty
    mat_prop = 'PorousFlow_fluid_phase_enthalpy_qp0'
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  csv = true
[]
