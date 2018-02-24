[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  nz = 0
  xmax = 20
  ymax = 16
  zmax = 0
  elem_type = QUAD4
  uniform_refine = 3
  parallel_type = replicated # This is because of floating point roundoff being different with DistributedMesh
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u
    coef = 1e-5
  [../]

  [./conv]
    type = Convection
    variable = u
    velocity = '-0.4 0 0'
  [../]

  [./forcing]
    type = GaussContForcing
    variable = u
    x_center = 6.0
    y_center = 8.0
    x_spread = 1.0
    y_spread = 2.0
  [../]

  [./dot]
    type = TimeDerivative
    variable = u
  [../]
[]

[BCs]
  [./Periodic]
    [./x]
      variable = u
      primary = 3
      secondary = 1
      translation = '20 0 0'
    [../]

    [./y]
      variable = u
      primary = 0
      secondary = 2
      translation = '0 16 0'
    [../]
  [../]
[]

[Executioner]
  type = Transient
  dt = 2
  num_steps = 7

  [./Adaptivity]
    refine_fraction = .80
    coarsen_fraction = .2
    max_h_level = 4
    error_estimator = KellyErrorEstimator
  [../]
[]

[Outputs]
  exodus = true
[]
