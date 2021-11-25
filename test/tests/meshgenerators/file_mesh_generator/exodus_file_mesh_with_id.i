[Mesh]
  [material_id_mesh]
    type = FileMeshGenerator
    file = mesh_with_material_id.e
    exodus_extra_element_integers = material_id
  []
[]

[AuxVariables]
  [material_id]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [set_material_id]
    type = ExtraElementIDAux
    variable = material_id
    extra_id_name = material_id
  []
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
