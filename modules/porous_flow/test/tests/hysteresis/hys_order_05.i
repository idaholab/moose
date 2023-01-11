# Test that PorousFlowHysteresisOrder correctly calculates hysteresis order
# Hysteresis order is initialised = 2, with turning points = (0.6, 0.8)
# Initial saturation is 0.71
# Water is removed from the system (so order = 2) until saturation = 0.6
# Then, water is removed from the system (so order = 0) until saturation = 0.58
# Then, water is added to the system (so order = 1 and turning point = 0.58) until saturation = 0.9
# Then, water is removed from the system (so order = 2 and turning point = 0.9)
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
    initial_condition = -9E5
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
    initial_order = 2
    previous_turning_points = '0.6 0.8'
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
    expression = '30 * if(t <= 2, -1, if(t <= 7, 1, -1))'
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
  end_time = 10
  nl_abs_tol = 1E-7
[]

[Outputs]
  [csv]
    type = CSV
  []
[]
