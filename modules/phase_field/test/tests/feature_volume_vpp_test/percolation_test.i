# This tests the percolation detection capability in FeatureFloodCount. One feature
# exists that intersects both left and right boundaries, so the FeatureVolumeVPP
# will return true for that feature based on the specified values of parameters
# primary_percolation_boundaries and secondary_percolation_boundaries.
# It also tests the capabilility of FeatureFloodCount to detect whether each feature
# is in contact with the boundaries set by the specified_boundaries parameter.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 60
  ny = 60
  xmin = 0
  xmax = 50
  ymin = 0
  ymax = 50
  elem_type = QUAD4
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
    type = MultiSmoothCircleIC
    variable = c
    invalue = 1.0
    outvalue = 0.0001
    radius = 4.0
    int_width = 2.0
    numbub = 35
    bubspac = 2
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
    primary_percolation_boundaries = 'left'
    secondary_percolation_boundaries = 'right'
    specified_boundaries = 'left right'
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
