#
# This test validates the correct application of the chain rule to coupled
# material properties within DerivativeParsedMaterials
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
[]

[Variables]
  [./eta1]
  [../]
  [./eta2]
  [../]
[]

[BCs]
  [./left]
    variable = eta1
    boundary = left
    type = DirichletBC
    value = 0
  [../]
  [./right]
    variable = eta1
    boundary = right
    type = DirichletBC
    value = 1
  [../]
  [./top]
    variable = eta2
    boundary = top
    type = DirichletBC
    value = 0
  [../]
  [./bottom]
    variable = eta2
    boundary = bottom
    type = DirichletBC
    value = 1
  [../]
[]

[Materials]
  # T1 := (eta1+1)^4
  [./term]
    type = DerivativeParsedMaterial
    property_name= T1
    coupled_variables = 'eta1'
    expression = '(eta1+1)^4'
    derivative_order = 4
  [../]

  # in this material we substitute T1 explicitly
  [./full]
    type = DerivativeParsedMaterial
    coupled_variables = 'eta1 eta2'
    property_name = F1
    expression = '(1-eta2)^4+(eta1+1)^4'
  [../]
  # in this material we utilize the T1 derivative material property
  [./subs]
    type = DerivativeParsedMaterial
    coupled_variables = 'eta1 eta2'
    property_name = F2
    expression = '(1-eta2)^4+T1'
    material_property_names = 'T1(eta1)'
  [../]

  # calculate differences between the explicit and indirect substitution version
  # the use if the T1 property should include dT1/deta1 contributions!
  # This also demonstrated the explicit use of material property derivatives using
  # the D[...] syntax.
  [./diff0]
    type = ParsedMaterial
    property_name = D0
    expression = '(F1-F2)^2'
    material_property_names = 'F1 F2'
  [../]
  [./diff1]
    type = ParsedMaterial
    property_name = D1
    expression = '(dF1-dF2)^2'
    material_property_names = 'dF1:=D[F1,eta1] dF2:=D[F2,eta1]'
  [../]
  [./diff2]
    type = ParsedMaterial
    property_name = D2
    expression = '(d2F1-d2F2)^2'
    material_property_names = 'd2F1:=D[F1,eta1,eta1] d2F2:=D[F2,eta1,eta1]'
  [../]

  # check that explicitly pulling a derivative yields the correct result by
  # taking the difference of the manually calculated 1st derivative of T1 and the
  # automatic derivative dT1 pulled in through dT1:=D[T1,eta1]
  [./diff3]
    type = ParsedMaterial
    property_name = E0
    expression = '(dTd1-(4*(eta1+1)^3))^2'
    coupled_variables = eta1
    material_property_names = 'dTd1:=D[T1,eta1]'
  [../]
[]

[Kernels]
  [./eta1diff]
    type = Diffusion
    variable = eta1
  [../]
  [./eta2diff]
    type = Diffusion
    variable = eta2
  [../]
[]

[Postprocessors]
  [./D0]
    type = ElementIntegralMaterialProperty
    mat_prop = D0
  [../]
  [./D1]
    type = ElementIntegralMaterialProperty
    mat_prop = D1
  [../]
  [./D2]
    type = ElementIntegralMaterialProperty
    mat_prop = D2
  [../]
  [./E0]
    type = ElementIntegralMaterialProperty
    mat_prop = E0
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  l_tol = 1e-03
[]

[Outputs]
  execute_on = 'TIMESTEP_END'
  csv = true
  print_linear_residuals = false
[]
