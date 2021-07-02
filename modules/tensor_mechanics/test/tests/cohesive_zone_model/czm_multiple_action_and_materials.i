#
# Stretch + rotation test
#
# This test is designed to compute a uniaxial stress and then follow it as the mesh is rotated by 45 degrees.
#
# The mesh is composed of two, single-elemnt blocks
# The large deforamtion traction separation kinematic assumes linear rotations and uses the velocity gradient L to keep track of area changes, hence it converges to the proper solutoin in the limit of dt->0. Smaller the time step higher the accuracy.

[Mesh]
  [./msh]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 4
    zmin = 0
    zmax = 4
  [../]
  [./subdomain_id]
    type = ElementSubdomainIDGenerator
    input = msh
    subdomain_ids = '0 1 2 3'
  []
  [./split]
    type = BreakMeshByBlockGenerator
    input = subdomain_id
    split_interface = true
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Functions]
  [./stretch]
    type = PiecewiseLinear
    x = '0 1'
    y = '0 100'
  [../]
[]

[BCs]
  [./fix_x]
    type = DirichletBC
    preset = true
    value = 0.0
    boundary = left
    variable = disp_x
  [../]
  [./fix_y]
    type = DirichletBC
    preset = true
    value = 0.0
    boundary = bottom
    variable = disp_y
  [../]
  [./fix_z]
    type = DirichletBC
    preset = true
    value = 0.0
    boundary = back
    variable = disp_z
  [../]
  [./back_z]
    type = FunctionNeumannBC
    boundary = front
    variable = disp_z
    use_displaced_mesh = false
    function = stretch
  [../]
[]


[Modules/TensorMechanics/CohesiveZoneMaster]
  [./czm_ik_012]
    boundary = 'Block0_Block1 Block1_Block2'
    kinematic = TotalLagrangian
    base_name = 'czm_b012'
  [../]
  [./czm_ik_23]
    boundary = 'Block2_Block3'
    kinematic = TotalLagrangian
    base_name = 'czm_b23'
  [../]
[]

[Materials]
  # cohesive materials
  [./czm_3dc1]
    type = SalehaniIrani3DCTractionIncremental
    boundary = 'Block0_Block1 Block1_Block2'
    normal_gap_at_maximum_normal_traction = 1
    tangential_gap_at_maximum_shear_traction = 0.5
    maximum_normal_traction = 500
    maximum_shear_traction = 300
    base_name = 'czm_b012'
  [../]
  [./czm_3dc3]
    type = SalehaniIrani3DCTraction
    boundary = 'Block2_Block3'
    normal_gap_at_maximum_normal_traction = 4
    tangential_gap_at_maximum_shear_traction = 2
    maximum_normal_traction = 500
    maximum_shear_traction = 300
    base_name = 'czm_b23'
  [../]
  # bulk materials
  [./stress]
    type = ADComputeFiniteStrainElasticStress
  [../]
  [./elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 200e4
    poissons_ratio = 0.3
  [../]
[]

[Modules]
  [./TensorMechanics]
    [./Master]
      [./all]
        strain = FINITE
        add_variables = true
        use_finite_deform_jacobian = true
        use_automatic_differentiation = true
        generate_output = 'stress_xx stress_yy stress_zz stress_xy stress_yz stress_xz'
      [../]
    [../]
  [../]
[]

[Postprocessors]
  [./nonlin]
    type = NumNonlinearIterations
  [../]
  [./stress_zz_01]
    type = SideAverageValue
    variable = stress_zz
    boundary = Block0_Block1
  [../]
  [./stress_zz_12]
    type = SideAverageValue
    variable = stress_zz
    boundary = Block1_Block2
  [../]

  [./stress_zz_23]
    type = SideAverageValue
    variable = stress_zz
    boundary = Block2_Block3
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  # Executioner
  type = Transient

  solve_type = 'NEWTON'
  line_search = none
  petsc_options_iname = '-pc_type '
  petsc_options_value = 'lu'
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-6
  l_max_its = 20
  start_time = 0.0
  dt = 0.25
  dtmin = 0.25
  num_steps =1
[]

[Outputs]
  exodus = true
[]
