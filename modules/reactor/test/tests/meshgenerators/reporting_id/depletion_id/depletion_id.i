[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = depletion_id_in.e
    exodus_extra_element_integers = 'material_id pin_id assembly_id'
  []
  [depl_map]
    type = DepletionIDGenerator
    input = 'fmg'
    id_name = 'assembly_id pin_id'
    material_id_name = 'material_id'
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[AuxVariables]
  [depletion_id]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [set_depletion_id]
    type = ExtraElementIDAux
    variable = depletion_id
    extra_id_name = depletion_id
  []
[]

[Outputs]
  exodus = true
  execute_on = timestep_end
[]
