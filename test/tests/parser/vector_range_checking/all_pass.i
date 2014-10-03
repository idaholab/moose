[Mesh]
  type = GeneratedMesh
  dim = 3
[]

[Materials]
  [./vecrangecheck]
    type = VecRangeCheckMaterial
    block = 0
    rv3 = '1.1 2.2 3.3'
    iv3 = '1 2 3'
    rvp = '0.1 0.2 0.3 0.4'
    ivg = '2 1'
    rvl = '0.0 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1.0 1.1 1.2 1.3'
  [../]
[]

[Problem]
  type = FEProblem
  solve = false
  kernel_check = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  [./console]
    type = Console
    perf_log = true
    linear_residuals = true
  [../]
[]
