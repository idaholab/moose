time = 3153600000

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    xmin = -5
    xmax = 5
    ny = 100
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
    scaling = 1e4
  []
[]

[AuxVariables]
  [density]
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
    mass_fraction = mass_frac_CH4
    phase = 0
    compute_enthalpy = false
    compute_internal_energy = false
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
  [pressure]
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
  dtmax = 1e6
  nl_rel_tol = 1e-4
  nl_abs_tol = 1e-8
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1000
  []
[]

[VectorPostprocessors]
  [mass_frac_CH4]
    type = LineValueSampler
    start_point = '0 0 0'
    end_point = '0 -100 0'
    variable = mass_frac_CH4
    sort_by = y
    num_points = 500
  []
  [density]
    type = LineValueSampler
    start_point = '0 0 0'
    end_point = '0 -100 0'
    variable = density
    sort_by = y
    num_points = 500
  []
[]

[Outputs]
  # sync_times = '0 315360000 3153600000'
  file_base = CO2_CH4_${time}
  [CSV]
    type = CSV
    sync_only = true
  []
  exodus = true
[]
