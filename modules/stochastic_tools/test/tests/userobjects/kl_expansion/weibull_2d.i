# Weibull random fields from a Gaussian copula, with a constant scale and with a scale that
# trends with distance from the vertical centerline x = 0.5
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 20
    ny = 20
  []
[]

[AuxVariables]
  [weibull]
  []
  [weibull_trend]
  []
[]

[AuxKernels]
  [weibull]
    type = KLWeibullAux
    variable = weibull
    kl_user_object = kl_field
    shape = 8
    scale = 10
    execute_on = INITIAL
  []
  [weibull_trend]
    type = SigmoidTrendWeibullAux
    variable = weibull_trend
    kl_user_object = kl_field
    start_point = '0.5 0 0'
    end_point = '0.5 1 0'
    scale_max = 10
    scale_min = 3
    shape = 8
    midpoint_of_sigmoid = 0.1
    slope_at_midpoint = 10
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
