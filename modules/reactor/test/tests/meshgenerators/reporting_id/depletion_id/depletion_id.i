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

[Outputs]
  [out]
    type = Exodus
    execute_on = timestep_end
    output_extra_element_ids = true
    show_extra_element_ids = 'depletion_id'
  []
[]
