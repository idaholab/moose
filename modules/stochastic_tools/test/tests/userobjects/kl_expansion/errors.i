# Truncation parameters are intentionally omitted and supplied by the error tests
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
[]

[AuxVariables]
  [weibull]
  []
[]

[AuxKernels]
  [weibull]
    type = SigmoidTrendWeibullAux
    variable = weibull
    kl_user_object = kl_field
    start_point = '0.5 0 0'
    end_point = '0.5 1 0'
    scale_max = 10
    scale_min = 3
    shape = 8
    midpoint_of_sigmoid = 0.1
    execute_on = INITIAL
  []
[]

[UserObjects]
  [cov_x]
    type = KLExponentialCovariance
    variance = 1
    length_scale = 0.2
  []
  [cov_y]
    type = KLExponentialCovariance
    variance = 1
    length_scale = 0.2
  []
  [kl_field]
    type = KLExpansionUserObject
    lower_bounds = '0 0'
    upper_bounds = '1 1'
    n_grid = '5 5'
    covariance_functions = 'cov_x cov_y'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
