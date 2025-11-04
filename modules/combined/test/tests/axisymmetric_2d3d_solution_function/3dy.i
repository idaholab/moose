[GlobalParams]
  order = FIRST
  family = LAGRANGE
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
  [./hoop_stress]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[UserObjects]
  [./soln]
    type = SolutionUserObject
    mesh = 2d_out.e
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

[Physics/SolidMechanics/QuasiStatic]
  [./all]
    volumetric_locking_correction = true
    add_variables  = true
    incremental = true
    strain = FINITE
    eigenstrain_names = thermal_expansion
    generate_output = 'stress_xx stress_yy stress_zz vonmises_stress hydrostatic_stress'
  [../]
[]

[AuxKernels]
  [./t_soln_aux]
    type = FunctionAux
    variable = temp
    block = '1 2'
    function = soln_func_temp
  [../]
  [./hoop_stress]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    variable = hoop_stress
    scalar_type = HoopStress
    execute_on = timestep_end
  [../]
[]

[BCs]
  [./x_soln_bc]
    type = FunctionDirichletBC
    variable = disp_x
    preset = false
    boundary = '1 2'
    function = soln_func_disp_x
  [../]
  [./y_soln_bc]
    type = FunctionDirichletBC
    variable = disp_y
    preset = false
    boundary = '1 2'
    function = soln_func_disp_y
  [../]
  [./z_soln_bc]
    type = FunctionDirichletBC
    variable = disp_z
    preset = false
    boundary = '1 2'
    function = soln_func_disp_z
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    block = '1 2'
    youngs_modulus = 193.05e9
    poissons_ratio = 0.3
  [../]

  [./stress]
    type = ComputeFiniteStrainElasticStress
    block = '1 2'
  [../]

  [./thermal_expansion]
    type = ComputeThermalExpansionEigenstrain
    block = '1 2'
    thermal_expansion_coeff = 13e-6
    stress_free_temperature = 295.00
    temperature = temp
    eigenstrain_name = thermal_expansion
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
    max_rows = 25
  [../]
[]
