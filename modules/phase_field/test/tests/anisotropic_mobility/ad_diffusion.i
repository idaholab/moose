[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 15
  ny = 15
  xmax = 15.0
  ymax = 15.0
[]

[Variables]
  [./c]
    [./InitialCondition]
      type = CrossIC
      x1 = 0.0
      x2 = 30.0
      y1 = 0.0
      y2 = 30.0
    [../]
  [../]
[]

[Kernels]
  [./cres]
    type = ADMatAnisoDiffusion
    diffusivity = D
    variable = c
  [../]
  [./time]
    type = ADTimeDerivative
    variable = c
  [../]
[]

[Materials]
  [./D]
    type = ADConstantAnisotropicMobility
    tensor = '0.1 0 0
              0   1 0
              0   0 0'
    M_name = D
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'BDF2'
  solve_type = 'NEWTON'

  l_max_its = 30
  l_tol = 1.0e-4
  nl_max_its = 50
  nl_rel_tol = 1.0e-10

  dt = 10.0
  num_steps = 2
[]

[Outputs]
  exodus = true
  print_linear_residuals = true
  perf_graph = true
[]
