[Problem]
  kernel_coverage_check = false
  material_coverage_check = false
[]

[Mesh]
  [cartesian]
    type = CartesianMeshGenerator
    dim = 2
    dx = '0.5 4 1 4 0.5'
    ix = '2 2 2 2 2'
    dy = '0.3 10 1'
    iy = '2 2 2'
    subdomain_id = '1  2  3  4   5
                    6  7  8  9  10
                    11 12 13 14 15'
  []

  [add_side_left_left]
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = 6
    paired_block = 7
    new_boundary = left_left
    input = cartesian
  []

  [add_side_left_bottom]
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = 2
    paired_block = 7
    new_boundary = left_bottom
    input = add_side_left_left
  []

  [add_side_left_right]
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = 8
    paired_block = 7
    new_boundary = left_right
    input = add_side_left_bottom
  []

  [add_side_left_top]
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = 12
    paired_block = 7
    new_boundary = left_top
    input = add_side_left_right
  []

  [add_side_right_left]
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = 8
    paired_block = 9
    new_boundary = right_left
    input = add_side_left_top
  []

  [add_side_right_bottom]
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = 4
    paired_block = 9
    new_boundary = right_bottom
    input = add_side_right_left
  []

  [add_side_right_right]
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = 10
    paired_block = 9
    new_boundary = right_right
    input = add_side_right_bottom
  []

  [add_side_right_top]
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = 14
    paired_block = 9
    new_boundary = right_top
    input = add_side_right_right
  []
[]

[GrayDiffuseRadiation]
  [left]
    boundary = 'left_left left_right left_bottom left_top'
    emissivity = '0.8 0.8 0.9 0.5'
    n_patches = '2 2 2 2'
    temperature = temperature
    ray_tracing_face_order = SECOND
  []

  [right]
    boundary = 'right_left right_right right_bottom right_top'
    emissivity = '0.8 0.8 0.9 0.5'
    n_patches = '2 2 2 2'
    temperature = temperature
    ray_tracing_face_order = SECOND
  []
[]

[Variables]
  [temperature]
    block =  '1  2  3  4  5 6   8  10 11 12 13 14 15'
    initial_condition = 300
  []
[]

[Kernels]
  [conduction]
    type = HeatConduction
    variable = temperature
    diffusion_coefficient = 10
    block =  '1  2  3  4  5 6   8  10 11 12 13 14 15'
  []
[]

[BCs]
  [bottom]
    type = DirichletBC
    variable = temperature
    value = 300
    boundary = bottom
  []

  [top]
    type = DirichletBC
    variable = temperature
    value = 400
    boundary = top
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
