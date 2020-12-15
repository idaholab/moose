[Mesh]
  ## This file is where you split your mesh
  ## execution must NOT be --mes-only as we are saving element integers for later use
  [msh]
    type = FileMeshGenerator
    file = simple_diffusion_test_mesh_in.e
  []
  [split]
    input = msh
    type = BreakMeshByBlockGenerator
    write_fake_neighbor_list_to_file = true
    fake_neighbor_list_file_name = 'fake_neighbors_test_bmbb.csv'
  []
[]

[AuxVariables]
  [bmbb_element_id]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [set_material_id]
    type = ElemExtraIDAux
    variable = bmbb_element_id
    extra_id_name = bmbb_element_id
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
