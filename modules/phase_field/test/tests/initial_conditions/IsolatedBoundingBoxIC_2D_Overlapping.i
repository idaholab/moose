[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  xmin = 0
  xmax = 400
  ny = 10
  ymin = 0
  ymax = 100
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
    type = IsolatedBoundingBoxIC
    variable = c
    smaller_coordinate_corners = '100 20 0 130 25 0 350 40 0'
    larger_coordinate_corners = '150 30 0 300 80 0 360 80 0'
    inside = '0.2 0.5 0.8'
    outside = 1
    int_width = 5
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
