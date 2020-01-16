[GlobalParams]
  order = FIRST
  family = LAGRANGE
  volumetric_locking_correction = false
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  file = one_element.e
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
  [./vel_x]
  [../]
  [./vel_y]
  [../]
  [./vel_z]
  [../]
  [./accel_x]
  [../]
  [./accel_y]
  [../]
  [./accel_z]
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    add_variables = true
    strain = FINITE
  []
[]

[Kernels]
  [./inertia_x]
    type = InertialForce
    variable = disp_x
    velocity = vel_x
    acceleration = accel_x
    beta = 0.3025
    gamma = 0.6
  [../]
  [./inertia_y]
    type = InertialForce
    variable = disp_y
    velocity = vel_y
    acceleration = accel_y
    beta = 0.3025
    gamma = 0.6
  [../]
  [./inertia_z]
    type = InertialForce
    variable = disp_z
    velocity = vel_z
    acceleration = accel_z
    beta = 0.3025
    gamma = 0.6
  [../]
[]

[AuxKernels]
  [./accel_x]
    type = NewmarkAccelAux
    variable = accel_x
    displacement = disp_x
    velocity = vel_x
    beta = 0.3025
    execute_on = timestep_end
  [../]
  [./accel_y]
    type = NewmarkAccelAux
    variable = accel_y
    displacement = disp_y
    velocity = vel_y
    beta = 0.3025
    execute_on = timestep_end
  [../]
  [./accel_z]
    type = NewmarkAccelAux
    variable = accel_z
    displacement = disp_z
    velocity = vel_z
    beta = 0.3025
    execute_on = timestep_end
  [../]
  [./vel_x]
    type = NewmarkVelAux
    variable = vel_x
    acceleration = accel_x
    gamma = 0.6
    execute_on = timestep_end
  [../]
  [./vel_y]
    type = NewmarkVelAux
    variable = vel_y
    acceleration = accel_y
    gamma = 0.6
    execute_on = timestep_end
  [../]
  [./vel_z]
    type = NewmarkVelAux
    variable = vel_z
    acceleration = accel_z
    gamma = 0.6
    execute_on = timestep_end
  [../]
[]


[BCs]
  [./bottom_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]

  [./bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0.0
  [../]

  [./bottom_z]
    type = DirichletBC
    variable = disp_z
    boundary = 1
    value = 0.0
  [../]

  [./top_z]
    type = DirichletBC
    variable = disp_z
    boundary = 2
    value = 0.0
  [../]

  [./top_x]
    type = DirichletBC
    variable = disp_x
    boundary = 2
    value = 0.0
  [../]

  [./top_y]
    type = FunctionDirichletBC
    variable = disp_y
    preset = false
    boundary = 2
    function = pull
  [../]

[]

[Materials]

  [./constant]
    type = ComputeIsotropicElasticityTensor
    block = '1'
    youngs_modulus = 1.26e6
    poissons_ratio = .33
  [../]
  [./constant_stress]
    type = ComputeFiniteStrainElasticStress
    block = '1'
  [../]

  [./density]
    type = GenericConstantMaterial
    block = 1
    prop_names = 'density'
    prop_values = '0.00023832'
  [../]

[]


[Executioner]

  type = Transient
  # PETSC options
  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      4'
  line_search = 'none'

  start_time = 0
  end_time = 1
  dtmax = 0.1
  dtmin = 0.1

  # control for adaptive time steping
  [./TimeStepper]
    type = ConstantDT
    dt = 0.1
  [../]

[]

[Functions]
  [./pull]
    type = PiecewiseLinear
    x = '0.0 0.1      0.2     0.3    0.4        0.5      0.6   0.7      0.8    0.9    1.0'
    y = '0.0 0.000167 0.00133 0.0045 0.010667   0.020833 0.036 0.057167 0.0853 0.1215 0.16667'
    scale_factor = 1
  [../]
[]

[Postprocessors]
   [./_dt]
     type = TimestepSize
   [../]
   [./nonlinear_its]
     type = NumNonlinearIterations
   [../]

[]

[Outputs]
  exodus = true
[]
