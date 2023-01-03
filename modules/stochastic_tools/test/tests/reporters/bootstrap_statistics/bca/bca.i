[StochasticTools]
[]

[VectorPostprocessors]
  # Table 14.1 in Efron and Tibshirani, 1993
  [A]
    type = ConstantVectorPostprocessor
    value = '48 36 20 29 42 42 20 42 22 41 45 14 6 0 33 28 34 4 32 24 47 41 24 26 30 41'
    outputs = none
  []
  [B]
    type = ConstantVectorPostprocessor
    value = '42 33 16 39 38 36 15 33 20 43 34 22 7 15 34 29 41 13 38 25 27 41 28 14 28 40'
    outputs = none
  []
[]

[Reporters]
  # Reproduce Table 13.1 in Efron and Tibshirani, 1993
  [stats]
    type = StatisticsReporter
    vectorpostprocessors = 'A B'
    compute = 'mean'
    ci_method = 'bca'
    ci_levels = '0.025 0.05 0.1 0.16 0.5 0.84 0.9 0.95 0.975'
    ci_replicates = 1000
    ci_seed = 1980
  []
[]

[Outputs]
  execute_on = FINAL
  [out]
    type = JSON
  []
[]
