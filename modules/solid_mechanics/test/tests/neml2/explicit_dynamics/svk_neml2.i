[Models]
  [gl_strain]
    type = GreenLagrangeStrain
    deformation_gradient = 'deformation_gradient'
    strain = 'E'
  []
  [svk]
    type = LinearIsotropicElasticity
    coefficients = '1 0.3'
    coefficient_types = 'YOUNGS_MODULUS POISSONS_RATIO'
    strain = 'E'
    stress = 'S_pk2'
  []
  [pk2_r2]
    type = SR2ToR2
    input = 'S_pk2'
    output = 'neml2_stress'
  []
  [pk1]
    type = R2Multiplication
    A = 'deformation_gradient'
    B = 'neml2_stress'
    to = 'pk1'
  []
  [model]
    type = ComposedModel
    models = 'gl_strain svk pk2_r2 pk1'
    additional_outputs = 'neml2_stress'
  []
[]
