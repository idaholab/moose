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
    type = MatAnisoDiffusion
    diffusivity = D
    variable = c
  [../]
  [./time]
    type = TimeDerivative
    variable = c
  [../]
[]

[Materials]
  [./D]
    type = ConstantAnisotropicMobility
    tensor = '0.1 0 0
              0   1 0
              0   0 0'
    M_name = D
  [../]
[]

[Preconditioning]
  [./SMP]
   type = SMP
   full = true
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'BDF2'

  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_pc_type -pc_asm_overlap'
  petsc_options_value = 'asm         31      lu      1'

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
