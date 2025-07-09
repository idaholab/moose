# Simple example of a 2-phase situation with hysteretic capillary pressure.  Gas is added to, removed from, and added to the system in order to observe the hysteresis
# All liquid water exists in component 0
# All gas exists in component 1
[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    number_fluid_phases = 2
    number_fluid_components = 2
    porous_flow_vars = 'pp0 pp1'
  []
[]

[Variables]
  [pp0]
    initial_condition = 0
  []
  [pp1]
    initial_condition = 1E-4
  []
[]

[Kernels]
  [mass_conservation0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp0
  []
  [mass_conservation1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = pp1
  []
[]

[DiracKernels]
  [pump]
    type = PorousFlowPointSourceFromPostprocessor
    mass_flux = flux
    point = '0.5 0 0'
    variable = pp1
  []
[]

[AuxVariables]
  [massfrac_ph0_sp0]
    initial_condition = 1
  []
  [massfrac_ph1_sp0]
    initial_condition = 0
  []
  [sat0]
    family = MONOMIAL
    order = CONSTANT
  []
  [sat1]
    family = MONOMIAL
    order = CONSTANT
  []
  [hys_order]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [sat0]
    type = PorousFlowPropertyAux
    variable = sat0
    phase = 0
    property = saturation
  []
  [sat1]
    type = PorousFlowPropertyAux
    variable = sat1
    phase = 1
    property = saturation
  []
  [hys_order]
    type = PorousFlowPropertyAux
    variable = hys_order
    property = hysteresis_order
  []
[]

[FluidProperties]
  [simple_fluid] # same properties used for both phases
    type = SimpleFluidProperties
    bulk_modulus = 10 # so pumping does not result in excessive porepressure
  []
[]

[Materials]
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  []
  [temperature]
    type = PorousFlowTemperature
    temperature = 20
  []
  [massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = 'massfrac_ph0_sp0 massfrac_ph1_sp0'
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
  [hys_order_material]
    type = PorousFlowHysteresisOrder
  []
  [pc_calculator]
    type = PorousFlow2PhaseHysPP
    alpha_d = 10.0
    alpha_w = 7.0
    n_d = 1.5
    n_w = 1.9
    S_l_min = 0.1
    S_lr = 0.2
    S_gr_max = 0.3
    Pc_max = 12.0
    high_ratio = 0.9
    low_extension_type = quadratic
    high_extension_type = power
    phase0_porepressure = pp0
    phase1_porepressure = pp1
  []
[]

[Postprocessors]
  [flux]
    type = FunctionValuePostprocessor
  function = 'if(t <= 14, 10, if(t <= 25, -10, 10))'
  []
  [hys_order]
    type = PointValue
    point = '0 0 0'
    variable = hys_order
  []
  [sat0]
    type = PointValue
    point = '0 0 0'
    variable = sat0
  []
  [sat1]
    type = PointValue
    point = '0 0 0'
    variable = sat1
  []
  [pp0]
    type = PointValue
    point = '0 0 0'
    variable = pp0
  []
  [pp1]
    type = PointValue
    point = '0 0 0'
    variable = pp1
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -pc_factor_shift_type'
    petsc_options_value = ' lu       NONZERO'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 4
  end_time = 46
  nl_abs_tol = 1E-10
[]

[Outputs]
  csv = true
  sync_times = '13 14 15 24 25 25.5 26 27 28 29'
[]
