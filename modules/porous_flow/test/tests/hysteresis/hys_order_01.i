# Test that PorousFlowHysteresisOrder correctly calculates hysteresis order
# Water is removed from the system (so order = 0) until saturation = S0
# Then, water is added to the system (so order = 1) until saturation = S1
# Then, water is removed from the system (so order = 2)
# More water is removed from the system so that the saturation < S0 (so order = 0)
[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [pp]
    initial_condition = 0.0
  []
[]

[PorousFlowUnsaturated]
  porepressure = pp
  fp = simple_fluid
[]

[DiracKernels]
  [source_sink_0]
    type = PorousFlowPointSourceFromPostprocessor
    point = '0 0 0'
    mass_flux = sink_strength
    variable = pp
  []
  [source_sink_1]
    type = PorousFlowPointSourceFromPostprocessor
    point = '1 0 0'
    mass_flux = sink_strength
    variable = pp
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
    porosity = 1.0
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '0 0 0   0 0 0   0 0 0'
  []
  [hys_order]
    type = PorousFlowHysteresisOrder
  []
[]

[AuxVariables]
  [hys_order]
    family = MONOMIAL
    order = CONSTANT
  []
  [tp0]
    family = MONOMIAL
    order = CONSTANT
  []
  [tp1]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [hys_order]
    type = PorousFlowPropertyAux
    variable = hys_order
    property = hysteresis_order
  []
  [tp0]
    type = PorousFlowPropertyAux
    variable = tp0
    property = hysteresis_saturation_turning_point
    hysteresis_turning_point = 0
  []
  [tp1]
    type = PorousFlowPropertyAux
    variable = tp1
    property = hysteresis_saturation_turning_point
    hysteresis_turning_point = 1
  []
[]

[Functions]
  [sink_strength_fcn]
    type = ParsedFunction
    expression = '30 * if(t <= 4, -1, if(t <= 7, 1, -1))'
  []
[]

[Postprocessors]
  [sink_strength]
    type = FunctionValuePostprocessor
    function = sink_strength_fcn
    outputs = 'none'
  []
  [saturation]
    type = PointValue
    point = '0 0 0'
    variable = saturation0
  []
  [hys_order]
    type = PointValue
    point = '0 0 0'
    variable = hys_order
  []
  [tp0]
    type = PointValue
    point = '0 0 0'
    variable = tp0
  []
  [tp1]
    type = PointValue
    point = '0 0 0'
    variable = tp1
  []
[]

[Preconditioning]
  [basic]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  end_time = 13
  nl_abs_tol = 1E-7
[]

[Outputs]
  [csv]
    type = CSV
    sync_times = '0 1 5 6 7 8 9 10 11 13' # cut out t=12 because numerical roundoff might mean order is not reduced exactly at t=12
    sync_only = true
  []
[]
