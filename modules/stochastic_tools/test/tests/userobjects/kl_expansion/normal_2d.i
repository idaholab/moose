# Correlated Gaussian random field on a 2D mesh, sampled at nodes and elements
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 20
    ny = 20
  []
[]

[AuxVariables]
  [normal_nodal]
  []
  [normal_elem]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [normal_nodal]
    type = KLNormalAux
    variable = normal_nodal
    kl_user_object = kl_field
    mean = 1
    execute_on = INITIAL
  []
  [normal_elem]
    type = KLNormalAux
    variable = normal_elem
    kl_user_object = kl_field
    mean = 1
    execute_on = INITIAL
  []
[]

[UserObjects]
  [cov_x]
    type = KLSquaredExponentialCovariance
    variance = 1
    length_scale = 0.2
  []
  [cov_y]
    type = KLSquaredExponentialCovariance
    variance = 1
    length_scale = 0.2
  []
  [kl_field]
    type = KLExpansionUserObject
    lower_bounds = '0 0'
    upper_bounds = '1 1'
    n_grid = '40 40'
    covariance_functions = 'cov_x cov_y'
    variance_fraction = 0.99
    seed = 50
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
