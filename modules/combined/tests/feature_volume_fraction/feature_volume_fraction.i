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

[Postprocessors]
  [./feature_counter]
    type = FeatureFloodCount
    variable = u
    execute_on = initial
  [../]
  [./Volume]
    type = VolumePostprocessor
    execute_on = initial
  [../]
[]

[VectorPostprocessors]
  [./volume_fraction]
    type = FeatureVolumeFraction
    mesh_volume = Volume
    feature_volumes = feature_counter
    execute_on = initial
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 2
[]

[Outputs]
  exodus = true
  csv = true
[]
