[GlobalParams]
  displacements = "disp_x disp_y disp_z"
[]

[Mesh]
  [./msh]
    type = GeneratedMeshGenerator
    nx = 4
    ny = 6
    nz = 8
    xmin = -2.5
    xmax = 2.5
    ymin = -2
    ymax = 2
    zmin = -1.5
    zmax = 1.5
    dim = 3
  [../]
  [./subdomain_1]
    type = SubdomainBoundingBoxGenerator
    input = msh
    bottom_left = '-2.5 -2 -1'
    top_right = '2.5 0 0.5'
    block_id = 1
  []
  [./subdomain_2]
    type = SubdomainBoundingBoxGenerator
    input = subdomain_1
    bottom_left = '-2.5 0 -1'
    top_right = '2.5 2 0.5'
    block_id = 2
  []
  [./subdomain_3]
    type = SubdomainBoundingBoxGenerator
    input = subdomain_2
    bottom_left = '-2.5 -2 0.5'
    top_right = '1.25 2 1.5'
    block_id = 3
  []
  [./subdomain_4]
    type = SubdomainBoundingBoxGenerator
    input = subdomain_3
    bottom_left = '1.25 -2 0.5'
    top_right = '5 2 1.5'
    block_id = 4
  []
  [./subdomain_5]
    type = SubdomainBoundingBoxGenerator
    input = subdomain_4
    bottom_left = '-2.5 -2 -1.5'
    top_right = '1.25 2 -1'
    block_id = 3
  []
  [./subdomain_6]
    type = SubdomainBoundingBoxGenerator
    input = subdomain_5
    bottom_left = '1.25 -2 -1.5'
    top_right = '2.5 2 -1'
    block_id = 4
  []
  [./split]
    type = BreakMeshByBlockGenerator
    input = subdomain_6
  []
  [./node_set1]
    input = split
    type = ExtraNodesetGenerator
    coord = '1.25 0.66666667 1.125'
    new_boundary = fix_all
  []
  [./node_set2]
    input = node_set1
    type = ExtraNodesetGenerator
    coord = '1.25 0.66666667 0.75'
    new_boundary = fix_xy
  []
  [./node_set3]
    input = node_set2
    type = ExtraNodesetGenerator
    coord = '1.25 1.33333333 0.75'
    new_boundary = fix_z
  []
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = SMALL
    add_variables = true
    generate_output = 'stress_xx stress_yy stress_zz stress_yz stress_xz stress_xy'
  [../]
[]

[Modules/TensorMechanics/CohesiveZoneMaster]
  [./czm1]
    boundary = 'interface'
    displacements = 'disp_x disp_y disp_z'
  [../]
[]

[BCs]
  [./Periodic]
    [./x]
      variable = disp_x
      auto_direction = 'x y z'
    [../]
    [./y]
      variable = disp_y
      auto_direction = 'x y z'
    [../]
    [./z]
      variable = disp_z
      auto_direction = 'x y z'
    [../]
  [../]


  [./displacement_z]
    type = FunctionDirichletBC
    boundary = top
    variable = disp_y
    function = '0.01*t'
  [../]

  [./fix1_x]
    type = DirichletBC
    boundary = "fix_all"
    variable = disp_x
    value = 0
  [../]
  [./fix1_y]
    type = DirichletBC
    boundary = "fix_all"
    variable = disp_y
    value = 0
  [../]
  [./fix1_z]
    type = DirichletBC
    boundary = "fix_all"
    variable = disp_z
    value = 0
  [../]

  [./fix2_x]
    type = DirichletBC
    boundary = "fix_xy"
    variable = disp_x
    value = 0
  [../]
  [./fix2_y]
    type = DirichletBC
    boundary = "fix_xy"
    variable = disp_y
    value = 0
  [../]

  [./fix3_z]
    type = DirichletBC
    boundary = "fix_z"
    variable = disp_z
    value = 0
  [../]
[]

[Materials]
  [./Elasticity_tensor]
    type = ComputeElasticityTensor
    block = '1'
    fill_method = symmetric_isotropic
    C_ijkl = '0.3 0.5e8'
  [../]
  [./Elasticity_tensor2]
    type = ComputeElasticityTensor
    block = '2'
    fill_method = symmetric_isotropic
    C_ijkl = '0.3 0.5e7'
  [../]
  [./Elasticity_tensor3]
    type = ComputeElasticityTensor
    block = '3'
    fill_method = symmetric_isotropic
    C_ijkl = '0.3 1e8'
  [../]
  [./Elasticity_tensor4]
    type = ComputeElasticityTensor
    block = '4'
    fill_method = symmetric_isotropic
    C_ijkl = '0.3 0.5e9'
  [../]
  [./stress]
    type = ComputeLinearElasticStress
    block = '1 2 3 4'
  [../]
  [./czm_3dc]
    type = SalehaniIrani3DCTraction
    boundary = 'interface'
    normal_gap_at_maximum_normal_traction = 1
    tangential_gap_at_maximum_shear_traction = 0.5
    maximum_normal_traction = 100
    maximum_shear_traction = 70
    displacements = 'disp_x disp_y disp_z'
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Postprocessors]
  [./s11]
    type = ElementAverageValue
    variable = stress_xx
    execute_on = 'initial timestep_end'
  [../]
  [./s21]
    type = ElementAverageValue
    variable = stress_yz
    execute_on = 'initial timestep_end'
  [../]
  [./s31]
    type = ElementAverageValue
    variable = stress_xz
    execute_on = 'initial timestep_end'
  [../]
  [./s12]
    type = ElementAverageValue
    variable = stress_xy
    execute_on = 'initial timestep_end'
  [../]
  [./s22]
    type = ElementAverageValue
    variable = stress_yy
    execute_on = 'initial timestep_end'
  [../]
  [./s32]
    type = ElementAverageValue
    variable = stress_yz
    execute_on = 'initial timestep_end'
  [../]
  [./s13]
    type = ElementAverageValue
    variable = stress_xz
    execute_on = 'initial timestep_end'
  [../]
  [./s23]
    type = ElementAverageValue
    variable = stress_yz
    execute_on = 'initial timestep_end'
  [../]
  [./s33]
    type = ElementAverageValue
    variable = stress_zz
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'newton'
  line_search = none

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  l_max_its = 2
  l_tol = 1e-14
  nl_max_its = 10
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-6

  start_time = 0.0
  dt = 0.2
  end_time = 0.2
[]

[Outputs]
  exodus = true
  csv = true
[]
