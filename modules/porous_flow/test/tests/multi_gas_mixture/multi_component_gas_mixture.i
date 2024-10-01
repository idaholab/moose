time = 0.1

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    xmin = -5
    xmax = 5
    ny = 10
    ymin = -100
    ymax = 0
  []
  [top_box]
    type = ParsedSubdomainMeshGenerator
    input = gen
    combinatorial_geometry = 'y >= -50 & y <= 0'
    block_id = 1
    block_name = top_box
  []
  [bot_box]
    type = ParsedSubdomainMeshGenerator
    input = top_box
    combinatorial_geometry = 'y >= -100 & y < -50'
    block_id = 2
    block_name = bot_box
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[Variables]
  [pp_CO2]
  []
  [mass_frac_CH4]
  []
[]

[AuxVariables]
  [density]
    family = MONOMIAL
    order = FIRST
  []
  [T]
    family = MONOMIAL
    order = FIRST
  []
[]

[Kernels]
  [mass_CO2]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = pp_CO2
  []

  [adv_CO2]
    type = PorousFlowFullySaturatedDarcyFlow
    variable = pp_CO2
    fluid_component = 1
  []

  [diff_CO2]
    type = PorousFlowDispersiveFlux
    fluid_component = 1
    variable = pp_CO2
    disp_trans = 0
    disp_long = 0
  []

  [mass_CH4]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = mass_frac_CH4
  []

  [adv_CH4]
    type = PorousFlowFullySaturatedDarcyFlow
    fluid_component = 0
    variable = mass_frac_CH4
  []

  [diff_CH4]
    type = PorousFlowDispersiveFlux
    fluid_component = 0
    variable = mass_frac_CH4
    disp_trans = 0
    disp_long = 0
  []

[]

[AuxKernels]
  [density]
    type = PorousFlowPropertyAux
    variable = density
    property = density
    phase = 0
    execute_on = 'initial timestep_end'
  []

  [T]
    type = PorousFlowPropertyAux
    variable = T
    property = temperature
    phase = 0
    execute_on = 'initial timestep_end'
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
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1E-14 0 0   0 1E-14 0   0 0 1E-14'
  []
  [CO2]
    type = PorousFlowMultiComponentGasMixture
    fp = 'CH4 CO2'
    x = mass_frac_CH4
    phase = 0
  []
  [massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = mass_frac_CH4
  []

  [temperature]
    type = PorousFlowTemperature
    temperature = 313
  []
  [diff]
    type = PorousFlowDiffusivityConst
    diffusion_coeff = '1e-7 1e-7'
    tortuosity = 1
  []
  [relperm]
    type = PorousFlowRelativePermeabilityCorey
    n = 1
    phase = 0
  []
[]

[ICs]
  [mass_frac_CH4_bot]
    type = ConstantIC
    variable = mass_frac_CH4
    value = 0
    block = bot_box
  []
  [mass_frac_CH4_top]
    type = ConstantIC
    variable = mass_frac_CH4
    value = 1
    block = top_box
  []
  [pressure_bot]
    type = ConstantIC
    variable = pp_CO2
    value = 4e6
  []
[]

[Preconditioning]
  [basic]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap'
    petsc_options_value = ' asm      lu           NONZERO                   2'
  []
[]

[Executioner]
  type = Transient
  end_time = ${time}
  dtmax = 0.1
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-12
[]

[Postprocessors]
  [pressure]
    type = ElementIntegralVariablePostprocessor
    variable = pp_CO2
  []
  [temperature]
    type = ElementIntegralVariablePostprocessor
    variable = T
  []
  [density]
    type = ElementIntegralMaterialProperty
    mat_prop = 'PorousFlow_fluid_phase_density_qp0'
  []
  [viscosity]
    type = ElementIntegralMaterialProperty
    mat_prop = 'PorousFlow_viscosity_qp0'
  []
  [internal_energy]
    type = ElementIntegralMaterialProperty
    mat_prop = 'PorousFlow_fluid_phase_internal_energy_qp0'
  []
  [enthalpy]
    type = ElementIntegralMaterialProperty
    mat_prop = 'PorousFlow_fluid_phase_enthalpy_qp0'
  []
[]

[Outputs]
  csv = true
[]
