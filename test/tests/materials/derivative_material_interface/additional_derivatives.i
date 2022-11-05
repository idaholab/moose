#
# This test validates the correct application of the chain rule to coupled
# material properties within DerivativeParsedMaterials
#

[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [a]
  []
[]

[Materials]
  [term]
    type = DerivativeParsedMaterial
    property_name = F
    coupled_variables = 'a'
    expression = '(a*b*d*e)^3'
    material_property_names = 'b d:=c e'
    derivative_order = 2
    additional_derivative_symbols = 'e d'
    outputs = exodus
  []

  [const]
    type = GenericConstantMaterial
    prop_names = 'b c e'
    prop_values = '1 2 3'
  []
[]

[Kernels]
  [a]
    type = Diffusion
    variable = a
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  l_tol = 1e-03
[]

[Outputs]
  execute_on = 'TIMESTEP_END'
  exodus = true
  print_linear_residuals = false
[]
