# A 3x3x3 tetrahedral grid whose central interior node (valence ~24) is moved nearly onto a
# neighbouring face plane, creating a flat sliver shared by many tets. The edge collapse repairs it
# while keeping the surrounding high-valence neighborhood valid and conformal.
[Mesh]
  [g]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 3
    ny = 3
    nz = 3
    elem_type = TET4
  []
  [move]
    type = MoveNodeGenerator
    input = g
    node_id = '114'
    new_position = '0.5 0.5 0.335'
  []
  [repair]
    type = MeshRepairGenerator
    input = move
    fix_sliver_elements = true
  []
  [diagnostics]
    type = MeshDiagnosticsGenerator
    input = repair
    examine_non_conformality = ERROR
    examine_element_overlap = ERROR
    examine_element_volumes = ERROR
  []
[]
[Problem]
  solve = false
[]
[Executioner]
  type = Steady
[]
