time = 2e7

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    xmin = -5
    xmax = 5
    ny = 100
    ymin = 0
    ymax = 100
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 -9.81 0'
[]

[Variables]
  [pp_H2]
  []

  [mass_frac_CH4]
  []
[]

[AuxVariables]
  [density]
    family = MONOMIAL
    order = FIRST
  []
[]

[Kernels]
  [mass_H2]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = pp_H2
  []

  [adv_H2]
    type = PorousFlowFullySaturatedDarcyFlow
    variable = pp_H2
    fluid_component = 1
  []

  [diff_H2]
    type = PorousFlowDispersiveFlux
    fluid_component = 1
    variable = pp_H2
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
    porous_flow_vars = 'pp_H2 mass_frac_CH4'
    number_fluid_phases = 1
    number_fluid_components = 2
  []
[]

[FluidProperties]
  [H2]
    type = HydrogenFluidProperties
  []

  [CH4]
    type = MethaneFluidProperties
  []
[]

[Materials]
  [ps]
    type = PorousFlow1PhaseFullySaturated
    porepressure = pp_H2
  []

  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  []

  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1E-14 0 0   0 1E-14 0   0 0 1E-14'
  []

  # [H2]
  #   type = PorousFlowSingleComponentFluid
  #   fp = "H2 CH4"
  #   phase = 0
  # []

  [massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = mass_frac_CH4
  []

  [temperature]
    type = PorousFlowTemperature
    temperature = 323
  []

  [diff]
    type = PorousFlowDiffusivityConst
    diffusion_coeff = '0.0000726 0.0000726'
    tortuosity = 1
  []

  [relperm]
    type = PorousFlowRelativePermeabilityCorey
    n = 1
    phase = 0
  []

[]

[ICs]
  [mass_frac_CH4]
    type = BoundingBoxIC
    x1 = -5
    x2 = 5
    y1 = 50
    y2 = 100
    variable = mass_frac_CH4
    inside = 0
    outside = 1
  []

  [pressure]
    type = ConstantIC
    variable = pp_H2
    value = 4e6

  []
[]

[BCs]
  [constant_porepressure_top]
    type = DirichletBC
    variable = pp_H2
    value = 4e6
    boundary = top
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
  dtmax = 250000
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-12
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1000
  []
[]

[Postprocessors]

[]

[Outputs]
  file_base = H2_CH4_vertical_${time}
  csv = true
  exodus = true
[]
