[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 30
  ny = 30
  xmin = 0
  xmax = 50
  ymin = 0
  ymax = 50
  elem_type = QUAD4
[]

[Variables]
  [c]
    order = FIRST
    family = LAGRANGE
  []
[]

[ICs]
  [c]
    type = LatticeSmoothCircleIC
    variable = c
    invalue = 1.0
    outvalue = 0.0001
    circles_per_side = '3 2'
    pos_variation = 10.0
    radius = 4.0
    int_width = 5.0
    radius_variation_type = uniform
    avoid_bounds = false
  []
[]

[Postprocessors]
  [./flood_count]
    type = FeatureFloodCount
    variable = c

    # Must be turned out to build data structures necessary for FeatureVolumeVPP
    compute_var_to_feature_map = true
    threshold = 0.5
    outputs = none
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
