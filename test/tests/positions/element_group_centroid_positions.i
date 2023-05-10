[Mesh]
  # Need additional reductions in code for distributed
  parallel_type = replicated

  [fmg]
    type = FileMeshGenerator
    file = '../../../modules/reactor/test/tests/meshgenerators/reporting_id/depletion_id/depletion_id_in.e'
    exodus_extra_element_integers = 'material_id pin_id assembly_id'
  []
[]

[Positions]
  [all_mesh_blocks]
    type = ElementGroupCentroidPositions
    grouping_type = 'block'
  []
  [block_1]
    type = ElementGroupCentroidPositions
    block = 1
    grouping_type = 'block'
  []
  [block_and_one_id]
    type = ElementGroupCentroidPositions
    block = '1 2'
    extra_id_name = 'pin_id'
    extra_id = '1 2 4'
    grouping_type = 'block_and_extra_id'
  []
  [block_and_two_id]
    type = ElementGroupCentroidPositions
    block = '1 2'
    extra_id_name = 'assembly_id pin_id'
    extra_id = '1; 1 2 4'
    grouping_type = 'block_and_extra_id'
  []
  [block_and_three_id]
    type = ElementGroupCentroidPositions
    block = '1 2'
    extra_id_name = 'assembly_id pin_id material_id'
    extra_id = '0; 1 2 4 6; ;'
    grouping_type = 'block_and_extra_id'
  []
  [three_ids]
    type = ElementGroupCentroidPositions
    extra_id_name = 'assembly_id pin_id material_id'
    extra_id = '0; 1 2 4 6; ;'
    grouping_type = 'extra_id'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  [out]
    type = JSON
    execute_on = FINAL
    execute_system_information_on = none
  []
[]
