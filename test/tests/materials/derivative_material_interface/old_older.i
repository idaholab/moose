#
# This test validates the correct application of the chain rule to coupled
# material properties within DerivativeParsedMaterials
#

[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Variables]
  [dummy]
  []
[]

[Materials]
  [t_square]
    type = GenericFunctionMaterial
    prop_names = t2
    prop_values = 't^2'
  []

  [t2_old]
    type = ParsedMaterial
    property_name = t2_old
    expression = t2_old
    material_property_names = 't2_old:=Old[t2]'
  []
  [t2_older]
    type = ParsedMaterial
    property_name = t2_older
    expression = t2_older
    material_property_names = 't2_older:=Older[t2]'
  []
[]

[Executioner]
  type = Transient
  dt = 1.5
  num_steps = 5
[]

[Postprocessors]
  [t2]
    type = ElementAverageMaterialProperty
    mat_prop = t2
  []
  [t2_old]
    type = ElementAverageMaterialProperty
    mat_prop = t2_old
  []
  [t2_older]
    type = ElementAverageMaterialProperty
    mat_prop = t2_older
  []
[]

[Outputs]
  csv = true
  print_linear_residuals = false
[]
