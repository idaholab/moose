[Mesh]
  file = nodal_area_3D.e
[]

[Variables]
  [./dummy]
  [../]
[]

[AuxVariables]
  [./nodal_area]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./dummy]
    type = Diffusion
    variable = dummy
  [../]
[]

[UserObjects]
  [./nodal_area]
    type = NodalArea
    variable = nodal_area
    boundary = 1
    execute_on = 'initial timestep_end'
  [../]
[]

[BCs]
  [./dummy]
    type = DirichletBC
    variable = dummy
    boundary = 1
    value = 100
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -ksp_gmres_restart'
  petsc_options_value = 'jacobi   101'

  line_search = 'none'

  nl_abs_tol = 1e-11
  nl_rel_tol = 1e-10

  l_max_its = 20
[]

[Outputs]
  exodus = true
[]
