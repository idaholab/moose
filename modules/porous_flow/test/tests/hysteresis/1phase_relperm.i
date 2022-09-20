# Simple example of a 1-phase situation with hysteretic relative permeability.  Water is removed and added to the system in order to observe the hysteresis
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
    number_fluid_phases = 1
    number_fluid_components = 1
    porous_flow_vars = 'pp'
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    alpha = 10.0
    m = 0.33
  []
[]

[Variables]
  [pp]
    initial_condition = 0
  []
[]

[Kernels]
  [mass_conservation]
    type = PorousFlowMassTimeDerivative
    variable = pp
  []
[]

[DiracKernels]
  [pump]
    type = PorousFlowPointSourceFromPostprocessor
    mass_flux = flux
    point = '0.5 0 0'
    variable = pp
  []
[]

[AuxVariables]
  [sat]
    family = MONOMIAL
    order = CONSTANT
  []
  [relperm]
    family = MONOMIAL
    order = CONSTANT
  []
  [hys_order]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [sat]
    type = PorousFlowPropertyAux
    variable = sat
    property = saturation
  []
  [relperm]
    type = PorousFlowPropertyAux
    variable = relperm
    property = relperm
    phase = 0
  []
  [hys_order]
    type = PorousFlowPropertyAux
    variable = hys_order
    property = hysteresis_order
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
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
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [pc_calculator]
    type = PorousFlow1PhaseP
    capillary_pressure = pc
    porepressure = pp
  []
  [hys_order_material]
    type = PorousFlowHysteresisOrder
  []
  [relperm_material]
    type = PorousFlowHystereticRelativePermeabilityLiquid
    phase = 0
    S_lr = 0.1
    S_gr_max = 0.2
    m = 0.9
    liquid_modification_range = 0.9
  []
[]

[Postprocessors]
  [flux]
    type = FunctionValuePostprocessor
    function = 'if(t <= 5, -10, 10)'
  []
  [hys_order]
    type = PointValue
    point = '0 0 0'
    variable = hys_order
  []
  [sat]
    type = PointValue
    point = '0 0 0'
    variable = sat
  []
  [relperm]
    type = PointValue
    point = '0 0 0'
    variable = relperm
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 0.5
  end_time = 10
  nl_abs_tol = 1E-10
[]

[Outputs]
  csv = true
[]
