#
# Test the the getDefaultMaterialProperty in DerivativeMaterialInterface.
# This test should only pass, if the construction order of the Materials
# using this interface does not influence the outcome.
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 1
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 0.1
  elem_type = QUAD4
[]

[GlobalParams]
  derivative_order = 2
[]

[Variables]
  [./c]
    [./InitialCondition]
      type = FunctionIC
      function = x
    [../]
  [../]
[]

[Kernels]
  [./dummy1]
    type = ADDiffusion
    variable = c
  [../]
  [./dummy2]
    type = ADTimeDerivative
    variable = c
  [../]
[]

[Materials]
  # derivatives used both before and after being declared
  [./sum_a_1]
    type = ADDerivativeSumMaterial
    property_name = Fa1
    sum_materials = 'Fa'
    coupled_variables = 'c'
    outputs = exodus
  [../]
  [./free_energy_a]
    type = ADDerivativeParsedMaterial
    property_name = Fa
    coupled_variables = 'c'
    expression = 'c^4'
  [../]
  [./sum_a_2]
    type = ADDerivativeSumMaterial
    property_name = Fa2
    sum_materials = 'Fa'
    coupled_variables = 'c'
    outputs = exodus
  [../]

  # derivatives declared after being used
  [./sum_b_1]
    type = ADDerivativeSumMaterial
    property_name = Fb1
    sum_materials = 'Fb'
    coupled_variables = 'c'
    outputs = exodus
  [../]
  [./free_energy_b]
    type = ADDerivativeParsedMaterial
    property_name = Fb
    coupled_variables = 'c'
    expression = 'c^4'
  [../]

  # derivatives declared before being used
  [./free_energy_c]
    type = ADDerivativeParsedMaterial
    property_name = Fc
    coupled_variables = 'c'
    expression = 'c^4'
  [../]
  [./sum_c_2]
    type = ADDerivativeSumMaterial
    property_name = Fc2
    sum_materials = 'Fc'
    coupled_variables = 'c'
    outputs = exodus
  [../]

  # non-existing derivatives
  [./free_energy_d]
    type = ADParsedMaterial
    property_name = Fd
    coupled_variables = 'c'
    expression = 'c^4'
  [../]
  [./sum_d_1]
    type = ADDerivativeSumMaterial
    property_name = Fd1
    sum_materials = 'Fd'
    coupled_variables = 'c'
    outputs = exodus
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2

  solve_type = 'NEWTON'
  num_steps = 1
  dt = 1e-5
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
