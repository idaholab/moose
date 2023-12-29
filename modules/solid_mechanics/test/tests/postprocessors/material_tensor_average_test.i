[Mesh]
  [./msh]
    type = GeneratedMeshGenerator
    dim = 3
    xmax = 2
    ymax = 2
    zmax = 2
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Modules]
  [./TensorMechanics]
    [./Master]
      [./all]
        strain = FINITE
        add_variables = true
        generate_output = 'stress_zz'
      [../]
    [../]
  [../]
[]

[BCs]
  [./back_z]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0.0
  [../]
  [./left_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]
  [./move_front]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = front
    function = 't/10.'
  [../]
[]

[Materials]
  [./stress]
    type = ComputeFiniteStrainElasticStress
  [../]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric9
    C_ijkl = '1.5e6 0.75e6 0.75e6 1.5e6 0.75e6 1.5e6 0.375e6 0.375e6 0.375e6'
  [../]
[]

[Postprocessors]
  [./szz_avg]
    type =MaterialTensorAverage
    rank_two_tensor = stress
    index_i = 2
    index_j = 2
    use_displaced_mesh = true
  []
  [./szz_int]
    type =MaterialTensorIntegral
    rank_two_tensor = stress
    index_i = 2
    index_j = 2
    use_displaced_mesh = true
  []
  [./szz_avg_aux]
    type =ElementAverageValue
    variable = stress_zz
    use_displaced_mesh = true
  []
  [./szz_int_aux]
    type =ElementIntegralVariablePostprocessor
    variable = stress_zz
    use_displaced_mesh = true
  []
[]


[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type '
  petsc_options_value = lu
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-6
  l_max_its = 20
  start_time = 0.0
  dt = 0.2
  end_time = 1.0
[]

[Outputs]
  csv = true
[]
