[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[Variables]
  [temp]
    initial_condition = 1100
  []
[]

[Kernels]
  [heat]
    type = ADDiffusion
    variable = temp
  []
[]

[BCs]
  [temp]
    type = ADFunctionDirichletBC
    variable = temp
    boundary = 'left right'
    function = '100 * t + 100'
  []
[]

[Materials]
  [D]
    type = ADArrheniusMaterialProperty
    temperature = temp
    activation_energy = '0.5 0.1'
    frequency_factor = '5 3e-3'
    gas_constant = 8.617e-5
    property_name = D
    outputs = all
  []
  [D_exact]
    type = ParsedMaterial
    property_name = D_exact
    coupled_variables = temp
    constant_names = 'Q1 D01 Q2 D02 R'
    constant_expressions = '0.5 5 0.1 3e-3 8.617e-5'
    expression = 'D01 * exp(-Q1 / R / temp) + D02 * exp(-Q2 / R / temp)'
    outputs = all
  []
[]

[Executioner]
  type = Transient

  num_steps = 10
[]

[Postprocessors]
  [D]
    type = ElementAverageValue
    variable = D
  []
  [D_exact]
    type = ElementAverageValue
    variable = D_exact
  []
  [diff_D]
    type = DifferencePostprocessor
    value1 = 'D'
    value2 = 'D_exact'
    outputs = console
  []
[]

[Outputs]
  csv = true
[]
