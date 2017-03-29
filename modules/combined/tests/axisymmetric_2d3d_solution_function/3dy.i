[GlobalParams]
  order = FIRST
  family = LAGRANGE
  disp_x = disp_x
  disp_y = disp_y
  disp_z = disp_z
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  file = 3dy.e
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[AuxVariables]
  [./temp]
  [../]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./vonmises]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./hoop_stress]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./hydrostatic_stress]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[UserObjects]
  [./soln]
    type = SolutionUserObject
    mesh = gold/2d_out.e
    system_variables = 'disp_x disp_y temp'
  [../]
[]

[Functions]
  [./soln_func_temp]
    type = Axisymmetric2D3DSolutionFunction
    solution = soln
    from_variables = 'temp'
  [../]
  [./soln_func_disp_x]
    type = Axisymmetric2D3DSolutionFunction
    solution = soln
    from_variables = 'disp_x disp_y'
    component = 0
  [../]
  [./soln_func_disp_y]
    type = Axisymmetric2D3DSolutionFunction
    solution = soln
    from_variables = 'disp_x disp_y'
    component = 1
  [../]
  [./soln_func_disp_z]
    type = Axisymmetric2D3DSolutionFunction
    solution = soln
    from_variables = 'disp_x disp_y'
    component = 2
  [../]
[]

[SolidMechanics]
  [./solid]
    temp = temp
  [../]
[]

[AuxKernels]
  [./t_soln_aux]
    type = FunctionAux
    variable = temp
    block = '1 2'
    function = soln_func_temp
  [../]
  [./stress_xx]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xx
    index = 0
  [../]
  [./stress_yy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yy
    index = 1
  [../]
  [./stress_zz]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_zz
    index = 2
  [../]
  [./vonmises]
    type = MaterialTensorAux
    tensor = stress
    variable = vonmises
    quantity = vonmises
  [../]
  [./hoop_stress]
    type = MaterialTensorAux
    tensor = stress
    variable = hoop_stress
    quantity = hoop
    execute_on = timestep_end
    point1 = '0. 0. 0.'
    point2 = '0. 1. 0.'
  [../]
  [./hydrostatic_stress]
    type = MaterialTensorAux
    tensor = stress
    variable = hydrostatic_stress
    quantity = hydrostatic
    execute_on = timestep_end
  [../]
[]

[BCs]
  [./x_soln_bc]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = '1 2'
    function = soln_func_disp_x
  [../]
  [./y_soln_bc]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = '1 2'
    function = soln_func_disp_y
  [../]
  [./z_soln_bc]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = '1 2'
    function = soln_func_disp_z
  [../]
[]

[Materials]
  [./solid_mechanics1]
    type = Elastic
    block = '1 2'
    temp = temp
    youngs_modulus = 193.05e9
    poissons_ratio = 0.3
    thermal_expansion = 13e-6
    stress_free_temperature = 295.00
  [../]

  [./density]
    type = Density
    block = '1'
    density = 8000.0
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options = '-ksp_snes_ew'
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = ' 201                hypre    boomeramg      4'
  line_search = 'none'
  l_max_its = 25
  nl_max_its = 20
  nl_rel_tol = 1e-10
  l_tol = 1e-2

  start_time = 0.0
  dt = 1
  end_time = 1
  dtmin = 1
[]

[Outputs]
  file_base = 3dy_out
  exodus = true
  [./console]
    type = Console
    perf_log = true
    max_rows = 25
  [../]
[]
