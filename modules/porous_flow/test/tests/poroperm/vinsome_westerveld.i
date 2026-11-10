# Testing PorousFlowPorosityVinsomeWesterveld produces correct values.
# The temperature at the fracture face is held at T = 50, while the adjacent matrix starts at
# matrix_reference_temperature = 200, so theta = -150 is a step change at t = 0.
# porosity = porosity_ref - thermal_opening_coeff * I
# where I is the Vinsome-Westerveld approximation to the matrix heat content.
# The exact heat content for a step change is I = 2 * theta * sqrt(kappa * t / pi), which is
# also output for comparison.
# With dt = 3.15E6 the error at t = 3.15E7 is -5.1%.  The error decreases approximately linearly
# as dt decreases (-3.5% with dt = 3.15E5, -3.3% with dt = 3.15E3), but it does not go to zero:
# for a step change, the Vinsome-Westerveld profile itself gives I = (12/11) * theta * sqrt(kappa * t),
# which is 3.3% smaller in magnitude than the exact value at all times.  The error depends only on
# the number of time steps since the step change (t / dt), not on kappa or on the size of dt.
#
# Single time step, computed by hand (the vinsome_westerveld_one_step test runs this input file
# with dt = 4E6 and num_steps = 1):
#   theta = T - matrix_reference_temperature = 50 - 200 = -150,  theta_old = 0,  I_old = 0
#   kappa = 1E-6,  t = dt = 4E6
#   a = kappa * dt = 4
#   d = sqrt(kappa * t) / 2 = 1
#   p = (a * theta / d + I_old - d^3 * (theta - theta_old) / a) / (3 * d^2 + a)
#     = (4 - 1/4) * theta / 7 = 15 * theta / 28 = -80.357142857
#   I = I_old + a * (theta / d - p) = 4 * (theta - 15 * theta / 28) = 13 * theta / 7 = -278.57142857
#   porosity = 0.1 - 1E-5 * I = 0.10278571429
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
  [porosity_VW]
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
