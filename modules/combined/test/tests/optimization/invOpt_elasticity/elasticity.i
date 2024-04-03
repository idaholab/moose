[Models]
  [adjoint_elasticity_model]
    type = LinearIsotropicElasticity
    youngs_modulus = 7.5
    poisson_ratio = 0.25
    strain = 'forces/E'
  []
  [forward_elasticity_model]
    type = LinearIsotropicElasticity
    youngs_modulus = 7.5
    poisson_ratio = 0.25
    strain = 'forces/E'
  []
[]
