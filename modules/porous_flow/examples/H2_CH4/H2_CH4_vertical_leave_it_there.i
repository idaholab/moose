time = 3e7

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
    initial_condition = 4e6
  []

  [pp_CH4]
    initial_condition = 4e6
  []

[]

[AuxVariables]
  [massfrac_H2]
    family = MONOMIAL
    order = FIRST
  []
  [massfrac_CH4]
    family = MONOMIAL
    order = FIRST
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp_H2 pp_CH4'
    number_fluid_phases = 2
    number_fluid_components = 2
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    m = 0.5
    alpha = 1e-4
    sat_lr = 0.3
    pc_max = 1e9
    log_extension = true
  []
[]

[Kernels]
  [mass_der_H2]
    type = PorousFlowMassTimeDerivative
    variable = pp_H2
  []

  [adv_H2]
    type = PorousFlowAdvectiveFlux
    variable = pp_H2
  []

  [diff_H2]
    type = PorousFlowDispersiveFlux
    variable = pp_H2
    disp_trans = '0 0'
    disp_long = '0 0'
  []

  [mass_der_CH4]
    type = PorousFlowMassTimeDerivative
    variable = pp_CH4
    fluid_component = 1
  []

  [adv_CH4]
    type = PorousFlowAdvectiveFlux
    variable = pp_CH4
    fluid_component = 1
  []

  [diff_CH4]
    type = PorousFlowDispersiveFlux
    variable = pp_CH4
    disp_trans = '0 0'
    disp_long = '0 0'
    fluid_component = 1
  []
[]

[FluidProperties]
  [CH4]
    type = MethaneFluidProperties
  []

  [H2]
    type = HydrogenFluidProperties
  []
[]

[Materials]
  [ps]
    type = PorousFlow2PhasePP
    phase0_porepressure = pp_H2
    phase1_porepressure = pp_CH4
    capillary_pressure = pc
  []

  [massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = 'massfrac_H2 massfrac_CH4'
  []

  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  []

  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1E-14 0 0   0 1E-14 0   0 0 1E-14'
  []

  [H2]
    type = PorousFlowSingleComponentFluid
    fp = H2
    phase = 0
  []

  [CH4]
    type = PorousFlowSingleComponentFluid
    fp = CH4
    phase = 1
  []

  [temperature]
    type = PorousFlowTemperature
    temperature = 323
  []

  [diff]
    type = PorousFlowDiffusivityConst
    diffusion_coeff = '0.0000726 0.0000726 0.0000726 0.0000726'
    tortuosity = '1 1'
  []

  [relperm_H2]
    type = PorousFlowRelativePermeabilityCorey
    n = 1
    phase = 0
  []

  [relperm_CH4]
    type = PorousFlowRelativePermeabilityCorey
    n = 1
    phase = 1
  []

[]

[BCs]

  # [constant_outlet_porepressure_top]
  #   type = DirichletBC
  #   variable = pp_H2
  #   value = 4e6
  #   boundary = top
  # []
[]

[ICs]
  [massfrac_CH4]
    type = BoundingBoxIC
    x1 = -5
    x2 = 5
    y1 = 50
    y2 = 100
    variable = massfrac_CH4
    inside = 0
    outside = 1
  []

  [massfrac_H2]
    type = BoundingBoxIC
    x1 = -5
    x2 = 5
    y1 = 50
    y2 = 100
    variable = massfrac_H2
    inside = 1
    outside = 0
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
  # [C]
  #   type = PointValue
  #   variable = C
  #   point = '0 25 0'
  # []

[]

[Outputs]
  file_base = H2_CH4_vertical_2phase_${time}
  csv = true
  exodus = true
[]
