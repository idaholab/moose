!include outside_grains.i

[Mesh]
  [renamed_boundary]
    type = RenameBlockGenerator
    input = boundary_mesh
    old_block = '1 2 3 4 5'
    new_block = '0 2 7 9 12'
  []
  [background_mesh]
    nx := 20
    ny := 20
  []
  [grains]
    boundary_mesh := renamed_boundary
  []
[]

[AuxVariables/element_id]
  block = grain3
[]

[Outputs]
  file_base = sparse_grain_ids_out
[]
