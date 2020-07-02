[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 50
    ny = 50
    xmin = 0
    xmax = 50
    ymin = 0
    ymax = 50
    elem_type = QUAD4
  []
  [./left_side]
    type = SubdomainBoundingBoxGenerator
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '24.9 50 0'
    input = gen
  [../]
  [./right_side]
    type = SubdomainBoundingBoxGenerator
    block_id = 2
    bottom_left = '25.1 0 0'
    top_right = '50 50 0'
    input = left_side
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

[AuxVariables]
  [./unique_regions]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[ICs]
  [./c]
    type = SpecifiedSmoothCircleIC
    variable = c
    invalue = 1.0
    outvalue = 0.0
    radii =       '4    5  10'
    x_positions = '25   25 25'
    y_positions = '37.5 25 0'
    z_positions = '0    0  0'
    int_width = 2.0
  []
[]

[Postprocessors]
  [./flood_count]
    type = FeatureFloodCount
    variable = c

    # Must be turned on to build data structures necessary for FeatureVolumeVPP
    compute_var_to_feature_map = true
    threshold = 0.001
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
    single_feature_per_element = false
  [../]
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = c
  []
[]

[AuxKernels]
  [./unique_regions]
    type = FeatureFloodCountAux
    variable = unique_regions
    flood_counter = flood_count
    field_display = UNIQUE_REGION
  [../]
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
