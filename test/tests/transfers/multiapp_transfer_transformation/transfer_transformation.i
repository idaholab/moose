[Mesh]
  [./gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 3
    ny = 5
    nz = 0
    xmax = 0.8
    xmin = 0.2
    zmin = 0
    zmax = 0
    elem_type = QUAD4
  []

  [./subdomain_id]
    type = ElementSubdomainIDGenerator
    input = gmg
    subdomain_ids = '0 1 2
                     0 1 2
                     0 1 2
                     0 1 2
                     0 1 2'
  []

  [./boundary01]
    type = SideSetsBetweenSubdomainsGenerator
    input = subdomain_id
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'boundary01'
  []

  [./boundary10]
    type = SideSetsBetweenSubdomainsGenerator
    input = boundary01
    primary_block = '1'
    paired_block = '0'
    new_boundary = 'boundary10'
  []

  [./boundary12]
    type = SideSetsBetweenSubdomainsGenerator
    input = boundary10
    primary_block = '1'
    paired_block = '2'
    new_boundary = 'boundary12'
  []

  [./boundary21]
    type = SideSetsBetweenSubdomainsGenerator
    input = boundary12
    primary_block = '2'
    paired_block = '1'
    new_boundary = 'boundary21'
  []

  uniform_refine = 3
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxVariables]
  [./fromsubelem]
    order = constant
    family = monomial
  [../]

  [./fromsub]
  []
[]

[BCs]
  [./left0]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]

  [./right0]
    type = DirichletBC
    variable = u
    boundary = boundary01
    value = 1
  [../]

  [./right1]
    type = DirichletBC
    variable = u
    boundary = boundary12
    value = 0
  [../]

  [./right2]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    app_type = MooseTestApp
    positions = '0.0  0.0  0.0'
    execute_on = 'timestep_end'
    input_files = transfer_transformation_sub.i
  []
[]

[Transfers]
  [from_sub]
    type = MultiAppGeometricInterpolationTransfer
    from_multi_app = sub
    num_points = 1
    shrink_gap_width = 0.2
    shrink_mesh = 'source'
    source_variable = 'u'
    variable = 'fromsub'
    exclude_gap_blocks = '1 3'
  []

  [from_sub_elem]
    type = MultiAppGeometricInterpolationTransfer
    from_multi_app = sub
    num_points = 4
    shrink_gap_width = 0.2
    shrink_mesh = 'source'
    source_variable = 'u'
    variable = 'fromsubelem'
    exclude_gap_blocks = '1 3'
  []

  [from_parent]
    type = MultiAppGeometricInterpolationTransfer
    to_multi_app = sub
    num_points = 1
    shrink_gap_width = 0.2
    shrink_mesh = 'target'
    source_variable = 'u'
    exclude_gap_blocks = '1 3'
    variable = 'fromparent'
  []

  [from_parent_elem]
    type = MultiAppGeometricInterpolationTransfer
    to_multi_app = sub
    num_points = 4
    shrink_gap_width = 0.2
    shrink_mesh = 'target'
    source_variable = 'u'
    exclude_gap_blocks = '1 3'
    variable = 'fromparentelem'
  []
[]
