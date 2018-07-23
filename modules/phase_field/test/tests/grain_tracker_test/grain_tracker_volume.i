# This test calculates the volume of a few simple shapes
# Using the FeatureVolumeVectorPostprocessor
[Mesh]
  # Required for use with distributed mesh
  type = GeneratedMesh
  dim = 2
  nx = 40
  ny = 40
  nz = 0
  xmin = -2
  xmax = 2
  ymin = -2
  ymax = 2
  zmax = 0
  elem_type = QUAD4
[]

[Variables]
  [./gr0]
  [../]
  [./gr1]
  [../]
[]

[ICs]
  [./circle]
    type = SmoothCircleIC
    x1 = 0
    y1 = 0
    radius = 1
    int_width = 0.01
    invalue = 1
    outvalue = 0
    variable = gr0
  [../]
  [./boxes]
    type = MultiBoundingBoxIC
    corners = '-1 -1 0
               0  0  0'
    opposite_corners = '-0.5 -0.5 0
                        1    1    0'
    inside = 1
    outside = 0
    variable = gr1
  [../]
[]

[Postprocessors]
  [./grain_tracker]
    type = GrainTracker
    variable = 'gr0 gr1'
    threshold = 0.1
    compute_var_to_feature_map = true
    execute_on = 'initial'
  [../]
  [./avg_feature_vol]
    type = AverageGrainVolume
    feature_counter = grain_tracker
    execute_on = 'initial'
  [../]
[]

[VectorPostprocessors]
  [./grain_volumes]
    type = FeatureVolumeVectorPostprocessor
    flood_counter = grain_tracker
    execute_on = 'initial'
  [../]
[]

[Executioner]
  type = Steady
  [./Adaptivity]
    initial_adaptivity = 3
    refine_fraction = 0.7
    coarsen_fraction = 0.1
    max_h_level = 3
  [../]
[]

[Problem]
  solve = false
[]

[Outputs]
  exodus = true
  csv = true
[]
