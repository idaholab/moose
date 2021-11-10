[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '0.5 0.5'
    dy = '1'
    ix = '5 5'
    iy = '10'
    subdomain_id = '1 1'
  []
  # Limited to 1 side to avoid inconsistencies in parallel
  [internal_sideset]
    type = ParsedGenerateSideset
    combinatorial_geometry = 'y<0.51 & y>0.49 & x<0.11'
    new_sideset_name = 'center'
    input = 'mesh'
  []
  # this keeps numbering continuous so tests dont fail on different ids in CSV
  allow_renumbering = false
[]

[Variables]
  [u]
  []
  [v]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [diff_v]
    type = Diffusion
    variable = v
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
  [left_v]
    type = DirichletBC
    variable = v
    boundary = left
    value = 1
  []
  [right_v]
    type = DirichletBC
    variable = v
    boundary = right
    value = 0
  []
[]

[VectorPostprocessors]
  inactive = 'internal_sample'
  [side_sample]
    type = SideValueSampler
    variable = 'u v'
    boundary = top
    sort_by = x
  []
  [internal_sample]
    type = SideValueSampler
    variable = 'u v'
    boundary = center
    sort_by = 'id'
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  execute_on = 'timestep_end'
  [vpp_csv]
    type = CSV
  []
[]
