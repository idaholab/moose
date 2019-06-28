[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 40
  ny = 40
  xmin = -15.0
  ymin = -15.0
  xmax = 15.0
  ymax = 15.0
[]

[Variables]
  [./c]
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = 0.0
      y1 = 0.0
      radius = 3
      int_width = 1
      invalue = 1
      outvalue = 0
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
    tensor = '.505 .495 .0
              .495 .505 .0
              .0   .0   .0'
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
  solve_type = 'NEWTON'
  scheme = bdf2

  petsc_options_iname = '-pc_type -sub_pc_type'
  petsc_options_value = 'asm      lu'

  l_max_its = 30
  l_tol = 1.0e-4
  nl_max_its = 50
  nl_rel_tol = 1.0e-10

  dt = 1.0
  num_steps = 20
[]

[Outputs]
  exodus = true
  perf_graph = true
[]
