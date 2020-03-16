[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Variables/u]
[]

[Surrogates/poly_chaos]
    type = PolynomialChaos
    filename = 'train_out_poly_chaos.rd'
[]

[VectorPostprocessors/pc_moments]
    type = PolyChaosStatistics
    pc_name = poly_chaos
    compute = 'mean stddev skewness kurtosis'
    execute_on = final
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Outputs/out]
    type = CSV
    execute_on = FINAL
[]
