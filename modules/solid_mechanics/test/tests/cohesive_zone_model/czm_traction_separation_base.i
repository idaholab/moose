# base test to check the implemantation traction separation laws
# Loads are expressed using functions. See the czm_materials/3DC section in the
# test file  for examples.
[Mesh]
  [./msh]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 1
    nz = 1
  []
  [./subdomain_1]
    type = SubdomainBoundingBoxGenerator
    input = msh
    bottom_left = '0 0 0'
    block_id = 1
    top_right = '0.5 1 1'
  []
  [./subdomain_2]
    type = SubdomainBoundingBoxGenerator
    input = subdomain_1
    bottom_left = '0.5 0 0'
    block_id = 2
    top_right = '1 1 1'
  []
  [./breakmesh]
    input = subdomain_2
    type = BreakMeshByBlockGenerator
  [../]
  [add_side_sets]
    input = breakmesh
    type = SideSetsFromNormalsGenerator
    normals = '0 -1  0
               0  1  0
               -1 0  0
               1  0  0
               0  0 -1
               0  0  1'
    fixed_normal = true
    new_boundary = 'y0 y1 x0 x1 z0 z1'
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
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
    strain = SMALL
    boundary = 'interface'
    generate_output = 'traction_x traction_y traction_z normal_traction tangent_traction jump_x jump_y jump_z normal_jump tangent_jump'
  [../]
[]

[BCs]
  [./left_x]
    type = DirichletBC
    variable = disp_x
    preset = false
    boundary = x0
    value = 0.0
  [../]
  [./left_y]
    type = DirichletBC
    variable = disp_y
    preset = false
    boundary = x0
    value = 0.0
  [../]
  [./left_z]
    type = DirichletBC
    variable = disp_z
    preset = false
    boundary = x0
    value = 0.0
  [../]
  [./right_x]
    type = FunctionDirichletBC
    variable = disp_x
    preset = false
    boundary = x1
  [../]
  [./right_y]
    type = FunctionDirichletBC
    variable = disp_y
    preset = false
    boundary = x1
  [../]
  [./right_z]
    type = FunctionDirichletBC
    variable = disp_z
    preset = false
    boundary = x1
  [../]
[]

[Materials]
  [./Elasticity_tensor]
    type = ComputeElasticityTensor
    block = '1 2'
    fill_method = symmetric_isotropic
    C_ijkl = '0.3 0.5e8'
  [../]
  [./stress]
    type = ComputeLinearElasticStress
    block = '1 2'
  [../]
  [./czm_mat]
    boundary = 'interface'
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
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  solve_type = NEWTON
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-6
  nl_max_its = 5
  l_tol = 1e-10
  l_max_its = 50
  start_time = 0.0
  dt = 0.2
  end_time = 3
  dtmin = 0.2
  line_search = none
[]

[Outputs]
  [./out]
    type = Exodus
  [../]
[]
