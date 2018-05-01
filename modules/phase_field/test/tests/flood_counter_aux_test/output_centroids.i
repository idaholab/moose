[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 1
  xmax = 4
  nx = 12
  ymin = 0
  ymax = 3
  ny = 12
[]

[Variables]
  [./c]
    [./InitialCondition]
      type = BoundingBoxIC
      x1 = 1.5
      x2 = 3
      y1 = 0
      y2 = 3
      inside = 1
      outside = 0
      variable = c
    [../]
  [../]
[]

[Kernels]
  [./dt]
    type = TimeDerivative
    variable = c
  [../]
[]

[Materials]
  [./needed]
    type = GenericConstantMaterial
    prop_names = 'const'
    prop_values = '1'
  [../]
[]

[Postprocessors]
  [./flood_count]
    type = FeatureFloodCount
    variable = c
    compute_var_to_feature_map = true
    threshold = 0.5
    outputs = none
  [../]
[]

[VectorPostprocessors]
  [./var_size]
    type = FeatureVolumeVectorPostprocessor
    flood_counter = flood_count
    output_centroids = true
    outputs = flood
  [../]
[]

[Outputs]
  [./flood]
    type = CSV
    execute_on = FINAL
  [../]
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 1
  solve_type = JFNK
  scheme = bdf2
[]
