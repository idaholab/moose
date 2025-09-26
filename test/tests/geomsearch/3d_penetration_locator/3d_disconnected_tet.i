[Mesh]
  [connected_mesh]
    type = FileMeshGenerator
    file = 3d_thermal_contact_tet.e
  []
  [exploded_mesh]
    type = BreakMeshByElementGenerator
    input = connected_mesh
    interface_name = 'interelement'
  []
  construct_side_list_from_node_list = true
  allow_renumbering = false # fix VPP ordering
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./gap_distance]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./l2]
    type = MassMatrix
    variable = u
    matrix_tags = 'system'
  [../]
[]

[AuxKernels]
  [./distance]
    type = PenetrationAux
    variable = gap_distance
    boundary = 'leftright'
    paired_boundary = 'rightleft'
    search_using_point_locator = true
  [../]
[]

[VectorPostprocessors]
  [gap_sampler]
    type = SideValueSampler
    boundary = 'leftright'
    variable = 'gap_distance'
    sort_by = 'id'
  []
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  csv = true
[]
