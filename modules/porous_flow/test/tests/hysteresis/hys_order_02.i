# Test that PorousFlowHysteresisOrder correctly calculates hysteresis order
# Water is removed from the system (so order = 0) until saturation = 0.55
# Then, water is added to the system (so order = 1) until saturation = 0.74
# Then, water is removed from the system (so order = 2) until saturation = 0.62
# Then, water is added to the system (so order = 3)
# Then, water is added to the system so that saturation exceeds 0.74, so order = 1
# Then, water is added to the system to saturation becomes 1, so order = 0
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
  [tp2]
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
  [tp2]
    type = PorousFlowPropertyAux
    variable = tp2
    property = hysteresis_saturation_turning_point
    hysteresis_turning_point = 2
  []
[]

[Functions]
  [sink_strength_fcn]
    type = ParsedFunction
    expression = '30 * if(t <= 7, -1, if(t <= 10, 1, if(t <= 12, -1, 1)))'
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
  [tp2]
    type = PointValue
    point = '0 0 0'
    variable = tp2
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
  end_time = 21
  nl_abs_tol = 1E-7
[]

[Outputs]
  [csv]
    type = CSV
    sync_times = '0 1 2 9 10 11 12 13 14 15 17 18 19 21' # cut out t=16 and t=20 because numerical roundoff might mean order is not reduced exactly at these times
    sync_only = true
  []
[]
