[GlobalParams]
  displacements = "disp_x disp_y"
[]

[Mesh]
  [./msh]
    type = GeneratedMeshGenerator
    nx = 4
    ny = 4
    xmin = -1
    xmax = 1
    ymin = -1
    ymax = 1
    dim = 2
  [../]
  [./subdomain_1]
    type = SubdomainBoundingBoxGenerator
    input = msh
    bottom_left = '-1 -1 0'
    top_right = '0 0 0'
    block_id = 1
  []
  [./subdomain_2]
    type = SubdomainBoundingBoxGenerator
    input = subdomain_1
    bottom_left = '-1 0 0'
    top_right = '0 1 0'
    block_id = 2
  []
  [./subdomain_3]
    type = SubdomainBoundingBoxGenerator
    input = subdomain_2
    bottom_left = '0 -1 0'
    top_right = '1 0 0'
    block_id = 3
  []
  [./subdomain_4]
    type = SubdomainBoundingBoxGenerator
    input = subdomain_3
    bottom_left = '0 0 0'
    top_right = '1 1 0'
    block_id = 4
  []
  [./split]
    type = BreakMeshByBlockGenerator
    input = subdomain_4
  []
  [./node_set1]
    input = split
    type = ExtraNodesetGenerator
    coord = '-0.5 0.5 0'
    new_boundary = fix_all
  []
  [./node_set2]
    input = node_set1
    type = ExtraNodesetGenerator
    coord = '0.5 0.5 0'
    new_boundary = fix_x
  []
  [./sidesets]
    type = SideSetsFromNormalsGenerator
    input = node_set2
    normals = '1  0  0
              -1  0  0
               0  1  0
               0 -1  0'
    fixed_normal = true
    new_boundary = 'x1 x0 y1 y0'
  []
  [./node_move]
    input = sidesets
    type = ExtraNodesetGenerator
    coord = '-0.5 -0.5 0'
    new_boundary = node_move
  []
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = SMALL
    add_variables = true
    generate_output = 'stress_xx stress_yy stress_xy'
  [../]
[]

[Modules/TensorMechanics/CohesiveZoneMaster]
  [./czm1]
    boundary = 'interface'
    displacements = 'disp_x disp_y'
  [../]
[]

[BCs]
  [./Periodic]
    [x_x]
      variable = disp_x
      primary  = x0
      secondary = x1
      translation = '2 0 0'
    []
    [y_x]
      variable = disp_y
      primary  = x0
      secondary = x1
      translation = '2 0 0'
    []
    [x_y]
      variable = disp_x
      primary  = y0
      secondary = y1
      translation = '0 2 0'
    []
    [y_y]
      variable = disp_y
      primary  = y0
      secondary = y1
      translation = '0 2 0'
    []
  [../]


  [./displacement_yt]
    type = FunctionDirichletBC
    boundary = node_move
    variable = disp_y
    function = '0.1*t'
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

  [./fix2_x]
    type = DirichletBC
    boundary = "fix_x"
    variable = disp_x
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
    maximum_normal_traction = 1000
    maximum_shear_traction = 700
    displacements = 'disp_x disp_y'
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
[]

[Executioner]
  type = Transient

  solve_type = 'newton'
  line_search = none

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'

  l_max_its = 2
  l_tol = 1e-14
  nl_max_its = 10
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10

  start_time = 0.0
  dt = 0.2
  end_time = 0.2
[]

[Outputs]
  exodus = true
  csv = true
  # nemesis = true
[]
