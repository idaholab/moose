# Simple example of a 2-phase situation with hysteretic relative permeability.  Gas is added to and removed from the system in order to observe the hysteresis
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
    porous_flow_vars = 'pp0 sat1'
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    alpha = 10.0
    m = 0.33
  []
[]

[Variables]
  [pp0]
  []
  [sat1]
    initial_condition = 0
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
    variable = sat1
  []
[]

[DiracKernels]
  [pump]
    type = PorousFlowPointSourceFromPostprocessor
    mass_flux = flux
    point = '0.5 0 0'
    variable = sat1
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
  [pp1]
    family = MONOMIAL
    order = CONSTANT
  []
  [hys_order]
    family = MONOMIAL
    order = CONSTANT
  []
  [relperm_liquid]
    family = MONOMIAL
    order = CONSTANT
  []
  [relperm_gas]
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
  [relperm_liquid]
    type = PorousFlowPropertyAux
    variable = relperm_liquid
    property = relperm
    phase = 0
  []
  [relperm_gas]
    type = PorousFlowPropertyAux
    variable = relperm_gas
    property = relperm
    phase = 1
  []
  [pp1]
    type = PorousFlowPropertyAux
    variable = pp1
    phase = 1
    property = pressure
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
  [pc_calculator]
    type = PorousFlow2PhasePS
    capillary_pressure = pc
    phase0_porepressure = pp0
    phase1_saturation = sat1
  []
  [hys_order_material]
    type = PorousFlowHysteresisOrder
  []
  [relperm_liquid]
    type = PorousFlowHystereticRelativePermeabilityLiquid
    phase = 0
    S_lr = 0.1
    S_gr_max = 0.2
    m = 0.9
    liquid_modification_range = 0.9
  []
  [relperm_gas]
    type = PorousFlowHystereticRelativePermeabilityGas
    phase = 1
    S_lr = 0.1
    S_gr_max = 0.2
    m = 0.9
    gamma = 0.33
    k_rg_max = 0.8
    gas_low_extension_type = linear_like
  []
[]

[Postprocessors]
  [flux]
    type = FunctionValuePostprocessor
    function = 'if(t <= 9, 10, -10)'
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
  [kr_liq]
    type = PointValue
    point = '0 0 0'
    variable = relperm_liquid
  []
  [kr_gas]
    type = PointValue
    point = '0 0 0'
    variable = relperm_gas
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
  dt = 0.5
  end_time = 18
  nl_abs_tol = 1E-10
[]

[Outputs]
  csv = true
[]
