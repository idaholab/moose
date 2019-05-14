[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  xmax = 20
  ymax = 16
  uniform_refine = 3
  parallel_type = distributed
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []

  [conv]
    type = Convection
    variable = u
    velocity = '0.4 0 0'
  []

  [forcing]
    type = GaussContForcing
    variable = u
    x_center = 6.0
    y_center = 8.0
    x_spread = 1.0
    y_spread = 2.0
  []

  [dot]
    type = TimeDerivative
    variable = u
  []
[]

[BCs]
  [all]
    type = DirichletBC
    variable = u
    boundary = 'bottom top left right'
    value = 0
  []
[]

[Executioner]
  type = Transient
  dt = 2
  num_steps = 10

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  [./Adaptivity]
    refine_fraction = .80
    coarsen_fraction = .2
    max_h_level = 4
    error_estimator = KellyErrorEstimator
  [../]
[]

[Outputs]
  nemesis = true
[]
