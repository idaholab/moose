# Testing PorousFlowPorosityVinsomeWesterveld with matrix_thickness when the fracture-face
# temperature changes sign repeatedly: theta = T - matrix_reference_temperature = t * sin(t).
# With porosity_ref = 0.5 and thermal_opening_coeff = 0.01, heat_content = (0.5 - porosity) / 0.01 = I_L,
# the heat content of the layer 0 <= z <= matrix_thickness = 0.5.  |I_L| must not exceed 0.5 * max|theta|.
# The exact I_L, from Duhamel's theorem, is computed by vinsome_westerveld_cyclic_exact.py
[GlobalParams]
  PorousFlowDictator = dictator
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    number_fluid_phases = 1
    number_fluid_components = 1
    porous_flow_vars = pp
  []
[]

[Variables]
  [pp]
    initial_condition = 1
  []
[]

[Kernels]
  [pp]
    type = TimeDerivative
    variable = pp
  []
[]

[AuxVariables]
  [T]
  []
  [porosity]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [T]
    type = FunctionAux
    variable = T
    function = 't * sin(t)'
  []
  [porosity]
    type = PorousFlowPropertyAux
    variable = porosity
    property = porosity
  []
[]

[Postprocessors]
  [theta]
    type = PointValue
    point = '0 0 0'
    variable = T
  []
  [porosity]
    type = PointValue
    point = '0 0 0'
    variable = porosity
  []
  [heat_content]
    type = ParsedPostprocessor
    pp_names = porosity
    expression = '(0.5 - porosity) / 0.01'
  []
[]

[Materials]
  [ps]
    type = PorousFlow1PhaseFullySaturated
    porepressure = pp
  []
  [temperature]
    type = PorousFlowTemperature
    temperature = T
  []
  [porosity_VW]
    type = PorousFlowPorosityVinsomeWesterveld
    porosity_ref = 0.5
    thermal_opening_coeff = 0.01
    matrix_thermal_diffusivity = 1
    matrix_reference_temperature = 0
    matrix_thickness = 0.5
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  end_time = 20
[]

[Outputs]
  csv = true
[]
