[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 40
  ny = 40
  nz = 0

  xmax = 40
  ymax = 40
  zmax = 0
  elem_type = QUAD4
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = 20
      y1 = 20
      radius = 10
      int_width = 1
      invalue = 1
      outvalue = 0
    [../]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./dot]
    type = TimeDerivative
    variable = u
  [../]
[]

[VectorPostprocessors]
  [./feature_volumes]
    type = FeatureVolumeVectorPostprocessor
    flood_counter = feature_counter
    execute_on = 'initial timestep_end'
    outputs = none
  [../]
[]

[Postprocessors]
  [./feature_counter]
    type = FeatureFloodCount
    variable = u
    compute_var_to_feature_map = true
    execute_on = 'initial timestep_end'
  [../]
  [./Volume]
    type = VolumePostprocessor
    execute_on = 'initial'
  [../]
  [./volume_fraction]
    type = FeatureVolumeFraction
    mesh_volume = Volume
    feature_volumes = feature_volumes
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 2
[]

[Outputs]
  csv = true
[]
