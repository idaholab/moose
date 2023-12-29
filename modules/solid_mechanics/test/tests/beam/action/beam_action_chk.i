# Test for checking syntax for line element action input.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  xmin = 0.0
  xmax = 1.0
  displacements = 'disp_x disp_y disp_z'
[]

[BCs]
  [./fixx1]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./fixy1]
    type = DirichletBC
    variable = disp_y
    boundary = left
    value = 0.0
  [../]
  [./fixz1]
    type = DirichletBC
    variable = disp_z
    boundary = left
    value = 0.0
  [../]
  [./fixr1]
    type = DirichletBC
    variable = rot_x
    boundary = left
    value = 0.0
  [../]
  [./fixr2]
    type = DirichletBC
    variable = rot_y
    boundary = left
    value = 0.0
  [../]
  [./fixr3]
    type = DirichletBC
    variable = rot_z
    boundary = left
    value = 0.0
  [../]
[]

[NodalKernels]
  [./force_1]
    type = ConstantRate
    variable = disp_y
    boundary = 2
    rate = 1e-2
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  line_search = 'none'
  nl_max_its = 15
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-8

  dt = 1
  dtmin = 1
  end_time = 2
[]

[Modules/TensorMechanics/LineElementMaster]
  [./block_1]
    add_variables = true

    # Geometry parameters
    Iy = 0.0141889
    Iz = 0.0141889
    y_orientation = '0.0 1.0 0.0'

    block = 1

    # dynamic simulation using consistent mass/inertia matrix
    dynamic_consistent_inertia=true

    #dynamic simulation using nodal mass/inertia matrix
    dynamic_nodal_translational_inertia = true

    dynamic_nodal_rotational_inertia = true
    nodal_Iyy = 1e-1
    nodal_Izz = 1e-1

    velocities = 'vel_x'
    accelerations = 'accel_x'
    rotational_accelerations = 'rot_accel_x'

    gamma = 0.5 # Newmark time integration parameter

    boundary = right # Node set where nodal mass and nodal inertia are applied

    # optional parameters for Rayleigh damping
    eta = 0.1 # Mass proportional Rayleigh damping
  [../]
  [./block_all]
    add_variables = true
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'

    # Geometry parameters
    area = 0.554256
    Iy = 0.0141889
    Iz = 0.0141889
    y_orientation = '0.0 1.0 0.0'
  [../]
[]

[Materials]
  [./stress]
    type = ComputeBeamResultants
    block = '1 2'
  [../]
  [./elasticity_1]
    type = ComputeElasticityBeam
    youngs_modulus = 2.0
    poissons_ratio = 0.3
    shear_coefficient = 1.0
    block = '1 2'
  [../]
[]

[Postprocessors]
  [./disp_y_1]
    type = PointValue
    point = '1.0 0.0 0.0'
    variable = disp_y
  [../]
  [./disp_y_2]
    type = PointValue
    point = '1.0 1.0 0.0'
    variable = disp_y
  [../]
[]

[Outputs]
  exodus = false
[]
