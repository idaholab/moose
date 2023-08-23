[Mesh]
  [dir1]
    type = ElementGenerator
    nodal_positions = '0 0 0
                     1 0 0
                     0 1 0'
    element_connectivity = '0 1 2'
    elem_type = 'TRI3'
  []
  [rename]
    type = RenameBlockGenerator
    input = dir1
    old_block = 0
    new_block = 1
  []
  [dir2]
    type = ElementGenerator
    nodal_positions = '0 0 0
                     1 0 0
                     0 1 0'
    element_connectivity = '0 2 1'
    elem_type = 'TRI3'
  []
  [combine]
    type = CombinerGenerator
    inputs = 'dir2 rename'
  []
  [extrude]
    type = MeshExtruderGenerator
    input = combine
    extrusion_vector = '0 0 1'
  []
  [flip]
    type = MeshRepairGenerator
    input = extrude
    fix_elements_orientation = true
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [vol0]
    type = VolumePostprocessor
    block = 0
  []
  [vol1]
    type = VolumePostprocessor
    block = 1
  []
[]

[Outputs]
  csv = true
[]
