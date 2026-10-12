# Testing PorousFlowPorosityVinsomeWesterveld produces correct values.
# The temperature at the fracture face is held at T = 50, while the adjacent matrix starts at
# matrix_reference_temperature = 200, so theta = -150 is a step change at t = 0.
# porosity = porosity_ref - thermal_opening_coeff * I
# where I is the Vinsome-Westerveld approximation to the matrix heat content.
# The exact heat content for a step change is I = 2 * theta * sqrt(kappa * t / pi), which is
# also output for comparison.
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
  [T]
    initial_condition = 50
  []
[]

[Kernels]
  [pp]
    type = TimeDerivative
    variable = pp
  []
  [T]
    type = TimeDerivative
    variable = T
  []
[]

[AuxVariables]
  [porosity]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [porosity]
    type = PorousFlowPropertyAux
    variable = porosity
    property = porosity
  []
[]

[Postprocessors]
  [porosity]
    type = PointValue
    point = '0 0 0'
    variable = porosity
  []
  [heat_content]
    type = ParsedPostprocessor
    pp_names = porosity
    expression = '(0.1 - porosity) / 1E-5'
  []
  [heat_content_exact]
    type = ParsedPostprocessor
    expression = '2 * (-150) * sqrt(1E-6 * t / pi)'
    constant_names = pi
    constant_expressions = 3.141592653589793
    use_t = true
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
  [porosity]
    type = PorousFlowPorosityVinsomeWesterveld
    porosity_ref = 0.1
    thermal_opening_coeff = 1E-5
    matrix_thermal_diffusivity = 1E-6
    matrix_reference_temperature = 200
  []
[]

[Executioner]
  type = Transient
  dt = 3.15E6
  num_steps = 10
[]

[Outputs]
  csv = true
[]
