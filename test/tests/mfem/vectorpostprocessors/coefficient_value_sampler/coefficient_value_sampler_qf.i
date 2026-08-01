!include coefficient_value_sampler.i

[QuadratureFunctions]
  [quadrature_coefficient]
    type = MFEMScalarQuadratureFunction
    coefficient = linear_coefficient
    order = 2
  []
[]

[VectorPostprocessors]
  [quadrature_sample]
    type = MFEMPointScalarCoefficientValueSampler
    coefficient = quadrature_coefficient
    points = '0.125 0.125 0'
  []
[]
