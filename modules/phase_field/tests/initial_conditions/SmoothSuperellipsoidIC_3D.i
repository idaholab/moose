[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 15
  ny = 15
  nz = 15
  xmax = 50
  ymax = 50
  zmax = 50
  elem_type = HEX8
[]

[Variables]
  [./c]
  [../]
[]

[ICs]
  [./c]
    type = SmoothSuperellipsoidIC
    variable = c
    x1 = 25.0
    y1 = 25.0
    z1 = 25.0
    a = 8
    b = 12
    c = 16
    n = 3.5
    invalue = 1.0
    outvalue = 0
    int_width = 4.0
  [../]
[]

[Kernels]
  [./ie_c]
    type = TimeDerivative
    variable = c
  [../]
  [./Diffusion]
    type = MatDiffusion
    variable = c
  [../]
[]

[BCs]
  [./Periodic]
    [./All]
      auto_direction = 'x y z'
    [../]
  [../]
[]

[Materials]
  [./Diffusivity]
    type = GenericConstantMaterial
    prop_names = D
    prop_values = 1.0
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2

  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 31'

  l_max_its = 20
  l_tol = 1.0e-5
  nl_max_its = 40
  nl_rel_tol = 5.0e-14

  start_time = 0.0
  num_steps = 1
  dt = 2.0
[]

[Outputs]
  exodus = true
[]
