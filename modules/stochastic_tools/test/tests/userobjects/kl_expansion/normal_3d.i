# Anisotropic correlated Gaussian random field on a 3D mesh
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 6
    ny = 6
    nz = 6
  []
[]

[AuxVariables]
  [normal]
  []
[]

[AuxKernels]
  [normal]
    type = KLNormalAux
    variable = normal
    kl_user_object = kl_field
    execute_on = INITIAL
  []
[]

[UserObjects]
  [cov_x]
    type = KLSquaredExponentialCovariance
    variance = 1
    length_scale = 0.3
  []
  [cov_y]
    type = KLExponentialCovariance
    variance = 1
    length_scale = 0.5
  []
  [cov_z]
    type = KLSquaredExponentialCovariance
    variance = 1
    length_scale = 0.3
  []
  [kl_field]
    type = KLExpansionUserObject
    lower_bounds = '0 0 0'
    upper_bounds = '1 1 1'
    n_grid = '20 20 20'
    covariance_functions = 'cov_x cov_y cov_z'
    variance_fraction = 0.95
    seed = 508
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
