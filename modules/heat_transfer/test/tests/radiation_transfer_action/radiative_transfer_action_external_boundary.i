[Problem]
  kernel_coverage_check = false
[]

[Mesh]
  [./cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1.3 1.9'
    ix = '3 3 3'
    dy = '6'
    iy = '9'
    subdomain_id = '0 1 2'
  [../]

    [./inner_left]
      type = SideSetsBetweenSubdomainsGenerator
      input = cmg
      primary_block = 0
      paired_block = 1
      new_boundary = 'inner_left'
    [../]

    [./inner_right]
      type = SideSetsBetweenSubdomainsGenerator
      input = inner_left
      primary_block = 2
      paired_block = 1
      new_boundary = 'inner_right'
    [../]

    [./inner_top]
      type = ParsedGenerateSideset
      combinatorial_geometry = 'abs(y - 6) < 1e-10'
      normal = '0 1 0'
      included_subdomain_ids = 1
      new_sideset_name = 'inner_top'
      input = 'inner_right'
    [../]

    [./inner_bottom]
      type = ParsedGenerateSideset
      combinatorial_geometry = 'abs(y) < 1e-10'
      normal = '0 -1 0'
      included_subdomain_ids = 1
      new_sideset_name = 'inner_bottom'
      input = 'inner_top'
    [../]

    [./rename]
      type = RenameBlockGenerator
      old_block = '2'
      new_block = '0'
      input = inner_bottom
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
    fixed_temperature_boundary = '6'
    fixed_boundary_temperatures = '800'
    view_factor_calculator = analytical
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = temperature
    boundary = left
    value = 1000
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
