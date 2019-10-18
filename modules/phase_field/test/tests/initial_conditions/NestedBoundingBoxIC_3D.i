[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
  xmin = 0
  xmax = 40
  ny = 10
  ymin = 0
  ymax = 30
  nz = 10
  zmin = 0
  zmax = 40
[]

[Problem]
  solve = false
[]

[Variables]
  [./c]
  [../]
[]

[ICs]
  [./c]
    type = NestedBoundingBoxIC
    variable = c
    smaller_coordinate_corners = '20 12 20 15 8 15 5 4 8'
    larger_coordinate_corners = '25 16 24 30 20 28 35 25 30'
    inside = '0.2 0.5 0.8'
    outside = 1
    int_width = 3
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -sub_pc_type   -sub_pc_factor_shift_type'
  petsc_options_value = 'asm       ilu            nonzero'
  l_max_its = 30
  nl_max_its = 10
  l_tol = 1.0e-4
  nl_rel_tol = 1.0e-10
  nl_abs_tol = 1.0e-11

  num_steps = 1
  dt = 1e-5
[]

[Outputs]
  exodus = true
[]
