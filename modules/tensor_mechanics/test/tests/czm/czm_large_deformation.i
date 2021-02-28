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
  nz = 2
  xmin = -0.5
  xmax = 0.5
  ymin = -0.5
  ymax = 0.5
  zmin = -1
  zmax = 1
  []
  [./new_block]
    type = SubdomainBoundingBoxGenerator
    input = msh
    block_id = 1
    bottom_left = '-0.5 -0.5 0'
    top_right = '0.5 0.5 0.5'
  []
  [./split]
    type = BreakMeshByBlockGenerator
    input = new_block
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Functions]
  [./stretch]
    type = PiecewiseLinear
    x = '0 1'
    y = '0 1e2'
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

  [./rotate_x]
    type = DisplacementAboutAxis
    boundary = 'front back'
    function = '90.'
    angle_units = degrees
    axis_origin = '0. 0. 0.'
    axis_direction = '0. 1. 0.'
    component = 0
    variable = disp_x
    angular_velocity = true
  [../]
  [./rotate_y]
    type = DisplacementAboutAxis
    boundary = 'front back'
    function = '90.'
    angle_units = degrees
    axis_origin = '0. 0. 0.'
    axis_direction = '0. 1. 0.'
    component = 1
    variable = disp_y
    angular_velocity = true
  [../]
  [./rotate_z]
    type = DisplacementAboutAxis
    boundary = 'front back'
    function = '90.'
    angle_units = degrees
    axis_origin = '0. 0. 0.'
    axis_direction = '0. 1. 0.'
    component = 2
    variable = disp_z
    angular_velocity = true
  [../]
[]

[AuxVariables]
  [./da_dA]
    family = MONOMIAL
    order = CONSTANT
  []
  [./tczm_X]
    family = MONOMIAL
    order = CONSTANT
  []
  [./tczm_Y]
    family = MONOMIAL
    order = CONSTANT
  []
  [./tczm_Z]
    family = MONOMIAL
    order = CONSTANT
  []
  [./TPK1czm_X]
    family = MONOMIAL
    order = CONSTANT
  []
  [./TPK1czm_Y]
    family = MONOMIAL
    order = CONSTANT
  []
  [./TPK1czm_Z]
    family = MONOMIAL
    order = CONSTANT
  []
[]
#
[AuxKernels]
  [./tczm_X]
    type = MaterialRealVectorValueAux
    boundary = 'interface'
    property = traction_global
    component = 0
    execute_on = 'TIMESTEP_END'
    variable = tczm_X
  []
  [./tczm_Y]
    type = MaterialRealVectorValueAux
    boundary = 'interface'
    property = traction_global
    component = 1
    execute_on = 'TIMESTEP_END'
    variable = tczm_Y
  []
  [./tczm_Z]
    type = MaterialRealVectorValueAux
    boundary = 'interface'
    property = traction_global
    component = 2
    execute_on = 'TIMESTEP_END'
    variable = tczm_Z
  []
  [./TPK1czm_X]
    type = MaterialRealVectorValueAux
    boundary = 'interface'
    property = PK1traction
    component = 0
    execute_on = 'TIMESTEP_END'
    variable = TPK1czm_X
  []
  [./TPK1czm_Y]
    type = MaterialRealVectorValueAux
    boundary = 'interface'
    property = PK1traction
    component = 1
    execute_on = 'TIMESTEP_END'
    variable = TPK1czm_Y
  []
  [./TPK1czm_Z]
    type = MaterialRealVectorValueAux
    boundary = 'interface'
    property = PK1traction
    component = 2
    execute_on = 'TIMESTEP_END'
    variable = TPK1czm_Z
  []
[]


[Modules/TensorMechanics/CohesiveZoneMaster]
  [./czm_ik]
    boundary = 'interface'
    kinematic = TotalLagrangian
  [../]
[]

[Controls]
  [./c1]
    type = TimePeriod
    enable_objects = 'BCs::fix_x BCs::fix_y BCs::fix_z BCs::back_z'
    disable_objects = 'BCs::rotate_x BCs::rotate_y BCs::rotate_z'
    start_time = '0'
    end_time = '1'
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


[Materials]
  [./stress]
    type = ADComputeFiniteStrainElasticStress
  [../]
  [./elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 1e3
    poissons_ratio = 0.3
  [../]
  [./czm_3dc]
    type = SalehaniIrani3DCTractionIncremental
    boundary = 'interface'
    normal_gap_at_maximum_normal_traction = 0.01
    tangential_gap_at_maximum_shear_traction = 0.005
    maximum_normal_traction = 1000
    maximum_shear_traction = 700
    displacements = 'disp_x disp_y disp_z'
  [../]
[]

[Postprocessors]
  [./nonlin]
    type = NumNonlinearIterations
  [../]
  [./stress_zz]
    type = SideAverageValue
    variable = stress_zz
    boundary = interface
  [../]
  [./stress_yy]
    type = SideAverageValue
    variable = stress_yy
    boundary = interface
  [../]
  [./stress_xx]
    type = SideAverageValue
    variable = stress_xx
    boundary = interface
  [../]
  [./stress_xy]
    type = SideAverageValue
    variable = stress_xy
    boundary = interface
  [../]
  [./stress_xz]
    type = SideAverageValue
    variable = stress_xz
    boundary = interface
  [../]
  [./stress_yz]
    type = SideAverageValue
    variable = stress_yz
    boundary = interface
  [../]
  [./tczm_X]
    type = SideAverageValue
    variable = tczm_X
    boundary = interface
  [../]
  [./tczm_Y]
    type = SideAverageValue
    variable = tczm_Y
    boundary = interface
  [../]
  [./tczm_Z]
    type = SideAverageValue
    variable = tczm_Z
    boundary = interface
  [../]
  [./TPK1czm_X]
    type = SideAverageValue
    variable = TPK1czm_X
    boundary = interface
  [../]
  [./TPK1czm_Y]
    type = SideAverageValue
    variable = TPK1czm_Y
    boundary = interface
  [../]
  [./TPK1czm_Z]
    type = SideAverageValue
    variable = TPK1czm_Z
    boundary = interface
  [../]
  [./Area]
    type = AreaPostprocessor
    boundary = interface
    use_displaced_mesh = true
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
  nl_rel_tol = 1e-30
  nl_abs_tol = 1e-10
  l_max_its = 20
  start_time = 0.0
  dt = 0.25
  end_time = 2.
[]

[Outputs]
  [./out]
    type = CSV
  [../]
[]
