# Vinsome-Westerveld porosity after a long cold pulse
# Example from the PorousFlowPorosityVinsomeWesterveld documentation: the fracture-face temperature
# change is theta = sin(2 pi (16 - t)^(1/4)), with kappa = 1.  Without matrix_thickness the matrix is semi-infinite;
# the tests also run it with matrix_thickness = 1 (an insulated layer).  With porosity_ref = 0.5 and
# thermal_opening_coeff = 0.01, heat_content = (0.5 - porosity) / 0.01 = I.  The tests run only the
# first 20 time steps; the documentation figure uses the full run to t = 15.  Exact values of I
# are computed by vinsome_westerveld_finite_exact.py
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
    function = 'sin(2 * pi * (16 - t)^0.25)'
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
  []
[]

[Executioner]
  type = Transient
  dt = 0.05
  end_time = 15
[]

[Outputs]
  csv = true
[]
