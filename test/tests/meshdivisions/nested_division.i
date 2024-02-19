[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = '../positions/depletion_id_in.e'
    exodus_extra_element_integers = 'material_id pin_id assembly_id'
  []
  # To keep VPP output consistently ordered
  allow_renumbering = false
[]

[MeshDivisions]
  [extra_id_div_1]
    type = ExtraElementIntegerDivision
    extra_id_name = 'material_id'
  []
  [extra_id_div_2]
    type = ExtraElementIntegerDivision
    extra_id_name = 'pin_id'
  []
  [nested_div]
    type = NestedDivision
    divisions = 'extra_id_div_2 extra_id_div_1'
  []
[]

[AuxVariables]
  [div]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [mesh_div]
    type = MeshDivisionAux
    variable = div
    mesh_division = 'nested_div'
  []
[]

[VectorPostprocessors]
  [div_out]
    type = ElementValueSampler
    variable = 'div'
    sort_by = 'id'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
