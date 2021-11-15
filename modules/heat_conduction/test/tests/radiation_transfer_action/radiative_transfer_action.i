[Problem]
  kernel_coverage_check = false
[]

[Mesh]
  type = MeshGeneratorMesh

  [./cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1.3 1.9'
    ix = '3 3 3'
    dy = '2 1.2 0.9'
    iy = '3 3 3'
    subdomain_id = '0 1 0
                    4 5 2
                    0 3 0'
  [../]

  [./inner_bottom]
    type = SideSetsBetweenSubdomainsGenerator
    input = cmg
    primary_block = 1
    paired_block = 5
    new_boundary = 'inner_bottom'
  [../]

  [./inner_left]
    type = SideSetsBetweenSubdomainsGenerator
    input = inner_bottom
    primary_block = 4
    paired_block = 5
    new_boundary = 'inner_left'
  [../]

  [./inner_right]
    type = SideSetsBetweenSubdomainsGenerator
    input = inner_left
    primary_block = 2
    paired_block = 5
    new_boundary = 'inner_right'
  [../]

  [./inner_top]
    type = SideSetsBetweenSubdomainsGenerator
    input = inner_right
    primary_block = 3
    paired_block = 5
    new_boundary = 'inner_top'
  [../]

  [./rename]
    type = RenameBlockGenerator
    old_block = '1 2 3 4'
    new_block = '0 0 0 0'
    input = inner_top
  [../]
[]

[Variables]
  [./temperature]
    block = 0
  [../]
[]

[Kernels]
  [./heat_conduction]
    type = HeatConduction
    variable = temperature
    block = 0
    diffusion_coefficient = 5
  [../]
[]

[GrayDiffuseRadiation]
  [./cavity]
    boundary = '4 5 6 7'
    emissivity = '0.9 0.8 0.4 1'
    n_patches = '2 2 2 3'
    partitioners = 'centroid centroid centroid centroid'
    centroid_partitioner_directions = 'x y y x'
    temperature = temperature
    adiabatic_boundary = '7'
    fixed_temperature_boundary = '4'
    fixed_boundary_temperatures = '1200'
    view_factor_calculator = analytical
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = temperature
    boundary = left
    value = 600
  [../]

  [./right]
    type = DirichletBC
    variable = temperature
    boundary = right
    value = 300
  [../]
[]

[Postprocessors]
  [./average_T_inner_right]
    type = SideAverageValue
    variable = temperature
    boundary = inner_right
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
