# Correlated Gaussian random field on a 1D mesh with a fixed number of KL terms
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 50
    xmax = 2
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
    mean = 2
    execute_on = INITIAL
  []
[]

[UserObjects]
  [cov_x]
    type = KLExponentialCovariance
    variance = 0.5
    length_scale = 0.4
  []
  [kl_field]
    type = KLExpansionUserObject
    lower_bounds = 0
    upper_bounds = 2
    n_grid = 60
    covariance_functions = cov_x
    n_terms = 20
    seed = 3
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
