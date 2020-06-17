[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 6
    ny = 25
    nz = 15
    xmin = 20
    xmax = 30
    ymin = 0
    ymax = 50
    zmin = 10
    zmax = 40
    elem_type = HEX8
  []
  [./left_side]
    input = gen
    type = SubdomainBoundingBoxGenerator
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '24.9 50 50'
  [../]
  [./right_side]
    input = left_side
    type = SubdomainBoundingBoxGenerator
    block_id = 2
    bottom_left = '25.1 0 0'
    top_right = '50 50 50'
  [../]
  [./iface_u]
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = 1
    paired_block = 2
    new_boundary = 10
    input = right_side
  [../]
[]

[Variables]
  [./c]
    order = FIRST
    family = LAGRANGE
  []
[]

[ICs]
  [./c]
    type = SpecifiedSmoothCircleIC
    variable = c
    invalue = 1.0
    outvalue = 0.0
    radii =       '4    5  10'
    x_positions = '25   25 25'
    y_positions = '40 25 0'
    z_positions = '25   25 25'
    int_width = 2.0
  []
[]

[Postprocessors]
  [./flood_count]
    type = FeatureFloodCount
    variable = c

    # Must be turned on to build data structures necessary for FeatureVolumeVPP
    compute_var_to_feature_map = true
    threshold = 0.5
    execute_on = INITIAL
  [../]
[]

[VectorPostprocessors]
  [./features]
    type = FeatureVolumeVectorPostprocessor
    flood_counter = flood_count

    # Turn on centroid output
    output_centroids = true
    execute_on = INITIAL
    boundary = 10
    single_feature_per_element = true
  [../]
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = c
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
  execute_on = INITIAL
[]
