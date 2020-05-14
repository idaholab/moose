[StochasticTools]
[]

[Surrogates/poly_chaos]
    type = PolynomialChaos
    filename = 'train_out_poly_chaos.rd'
[]

[VectorPostprocessors/pc_moments]
    type = PolynomialChaosStatistics
    pc_name = poly_chaos
    compute = 'mean stddev skewness kurtosis'
    execute_on = final
[]

[Outputs/out]
    type = CSV
    execute_on = FINAL
[]
