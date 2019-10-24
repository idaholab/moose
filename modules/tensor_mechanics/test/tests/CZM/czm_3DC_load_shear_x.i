[Mesh]
  [./msh]
    type = FileMeshGenerator
    file = coh3D_3Blocks.e
  []

  [./breakmesh]
    input = msh
    type = BreakMeshByBlockGenerator
  [../]

  [./bottom_block_1]
    input = breakmesh
    type = SideSetsAroundSubdomainGenerator
    block = '1'
    new_boundary = 'bottom_1'
    normal = '0 0 -1'
  [../]
  [./top_block_2]
    input = bottom_block_1
    type = SideSetsAroundSubdomainGenerator
    block = '2'
    new_boundary = 'top_2'
    normal = '0 0 1'
  [../]
  [./top_block_3]
    input = top_block_2
    type = SideSetsAroundSubdomainGenerator
    block = '3'
    new_boundary = 'top_3'
    normal = '0 0 1'
  [../]

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


[BCs]
  [./bottom_x]
    type = DirichletBC
    variable = disp_x
    boundary = bottom_1
    value = 0.0
  [../]
  [./bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom_1
    value = 0.0
  [../]
  [./bottom_z]
    type = DirichletBC
    variable = disp_z
    boundary = bottom_1
    value = 0.0
  [../]
  [./top2_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = top_2
    function = 1*t
  [../]
  [./top2_y]
    type = DirichletBC
    variable = disp_y
    boundary = top_2
    value = 0.0
  [../]
  [./top2_z]
    type = DirichletBC
    variable = disp_z
    boundary = top_2
    value = 0.0
  [../]
  [./top3_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = top_3
    function = 1*t
  [../]
  [./top3_y]
    type = DirichletBC
    variable = disp_y
    boundary = top_3
    value = 0.0
  [../]
  [./top3_z]
    type = DirichletBC
    variable = disp_z
    boundary = top_3
    value = 0.0
  [../]
[]

[InterfaceKernels]
  [./interface_x]
    type = CZMInterfaceKernel
    variable = disp_x
    neighbor_var = disp_x
    displacements = 'disp_x disp_y disp_z'
    disp_index = 0
    boundary = 'interface'
  [../]
  [./interface_y]
    type = CZMInterfaceKernel
    variable = disp_y
    neighbor_var = disp_y
    displacements = 'disp_x disp_y disp_z'
    disp_index = 1
    boundary = 'interface'
  [../]
  [./interface_z]
    type = CZMInterfaceKernel
    variable = disp_z
    neighbor_var = disp_z
    disp_index = 2
    displacements = 'disp_x disp_y disp_z'
    boundary = 'interface'
  [../]
[]

[Materials]
  [./Elasticity_tensor]
    type = ComputeElasticityTensor
    block = '1 2 3'
    fill_method = symmetric_isotropic
    C_ijkl = '0.3 0.5e8'
  [../]
  [./stress]
    type = ComputeLinearElasticStress
    block = '1 2 3'
  [../]
  [./czm_3dc]
    type = CZM_3DCMaterial
    boundary = 'interface'
    MaxAllowableTraction = '100 70'
    DeltaU0 = '1 0.7'
    displacements = 'disp_x disp_y disp_z'
  [../]
[]
 [Preconditioning]
   [./SMP]
     type = SMP
     full = true
   [../]
 []
[Executioner]
  # Preconditisoned JFNK (default)
  type = Transient
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  # petsc_options_value = 'hypre     boomerang'
  solve_type = NEWTON
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-6
  nl_max_its = 5
  l_tol = 1e-10
  l_max_its = 50
  start_time = 0.0
  dt = 0.1
  end_time = 3
  dtmin = 0.1
  line_search = none
  # num_steps = 1
[]
[Outputs]
  [./out]
    type = Exodus
    sync_times = 0.494974746830583
  [../]
[]
[Postprocessors]
  [./sxx_3G]
    type = ElementAverageValue
    variable = stress_xx
    execute_on = 'initial timestep_end'
    block = 3
  [../]
  [./syy_3G]
    type = ElementAverageValue
    variable = stress_yy
    execute_on = 'initial timestep_end'
    block = 3
  [../]
  [./szz_3G]
    type = ElementAverageValue
    variable = stress_zz
    execute_on = 'initial timestep_end'
    block = 3
  [../]
  [./syz_3G]
    type = ElementAverageValue
    variable = stress_yz
    execute_on = 'initial timestep_end'
    block = 3
  [../]
  [./sxz_3G]
    type = ElementAverageValue
    variable = stress_xz
    execute_on = 'initial timestep_end'
    block = 3
  [../]
  [./sxy_3G]
    type = ElementAverageValue
    variable = stress_xy
    execute_on = 'initial timestep_end'
    block = 3
  [../]
  [./disp_top3_x]
    type = SideAverageValue
    variable = disp_x
    execute_on = 'initial timestep_end'
    boundary = 'top_3'
  [../]
[]
