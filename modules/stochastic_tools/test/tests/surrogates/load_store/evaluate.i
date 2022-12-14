[StochasticTools]
[]

[Surrogates/poly_chaos]
    type = PolynomialChaos
    filename = 'train_out_poly_chaos.rd'
[]

[Reporters/pc_data]
    type = PolynomialChaosReporter
    pc_name = poly_chaos
    include_data = true
    execute_on = final
[]

[Outputs/out]
    type = JSON
    execute_on = FINAL
[]
