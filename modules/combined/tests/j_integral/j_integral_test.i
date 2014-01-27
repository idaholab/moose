#This tests the calculation for the j integral
#Currently under development
#With a refined mesh, the value for j should be 2.434
#Analytical value is 2.425
#National Agency for Finite Element Methods and Standards (U.K.): Test 1.1 from NAFEMS publication œôòü2D Test Cases in Linear Elastic Fracture Mechanics, œôòü R0020.
#Currently, the ElementIntegralPostprocessor returns
#different values for small strain J and large strain J
#Even when the input (Eshelby tensor) is almost identical
#More investigation required



[GlobalParams]
  order = FIRST
#  order = SECOND
  family = LAGRANGE
  disp_x = disp_x
  disp_y = disp_y
  disp_z = disp_z
[]

[Mesh]
  file = crack.e
  displacements = 'disp_x disp_y disp_z'
  partitioner = centroid
  centroid_partitioner_direction = z
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
  [./q1]
  [../]
  [./q2]
  [../]
  [./q3]
  [../]
  [./q4]
  [../]
  [./q5]
  [../]
  [./stress_xx]      # stress aux variables are defined for output; this is a way to get integration point variables to the output file
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./vonmises]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./SED]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]


[Functions]
  [./rampConstant]
    type = PiecewiseLinear
    x = '0. 1.'
    y = '0. 1.'
    scale_factor = -1e2
  [../]
[]

[UserObjects]
  [./crackFrontDefinition]
    type = CrackFrontDefinition
    boundary = 800
    execute_on = residual
    crack_direction = '1 0 0'
    2d = true
    2d_axis = 2
    use_displaced_mesh = false
  [../]
[]

[SolidMechanics]
  [./solid]
  [../]
[]

[AuxKernels]
  [./q1]
    type = qFunctionJIntegral
    variable = q1
    j_integral_radius_inner = 4
    j_integral_radius_outer = 4.5
    crack_front_definition = crackFrontDefinition
  [../]
  [./q2]
    type = qFunctionJIntegral
    variable = q2
    j_integral_radius_inner = 4.5
    j_integral_radius_outer = 5
    crack_front_definition = crackFrontDefinition
  [../]
  [./q]
    type = qFunctionJIntegral
    variable = q3
    j_integral_radius_inner = 5
    j_integral_radius_outer = 5.5
    crack_front_definition = crackFrontDefinition
  [../]
  [./q4]
    type = qFunctionJIntegral
    variable = q4
    j_integral_radius_inner = 5.5
    j_integral_radius_outer = 6
    crack_front_definition = crackFrontDefinition
  [../]
  [./q5]
    type = qFunctionJIntegral
    variable = q5
    j_integral_radius_inner = 6
    j_integral_radius_outer = 6.5
    crack_front_definition = crackFrontDefinition
  [../]
  [./stress_xx]               # computes stress components for output
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xx
    index = 0
    execute_on = timestep     # for efficiency, only compute at the end of a timestep
  [../]
  [./stress_yy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yy
    index = 1
    execute_on = timestep
  [../]
  [./stress_zz]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_zz
    index = 2
    execute_on = timestep
  [../]
  [./vonmises]
    type = MaterialTensorAux
    tensor = stress
    variable = vonmises
    quantity = vonmises
    execute_on = timestep
  [../]
  [./SED]
    type = MaterialRealAux
    variable = SED
    property = strain_energy_density
    execute_on = timestep
  [../]
[]

[BCs]

#  [./pin_x]
#    type = DirichletBC
#    variable = disp_x
#    boundary = 200
#    value = 0.0
#  [../]
# 
#  [./pin_y]
#    type = DirichletBC
#    variable = disp_y
#    boundary = 200
#    value = 0.0
#  [../]

  [./crack_y]
    type = DirichletBC
    variable = disp_y
    boundary = 100
    value = 0.0
  [../]

 # [./nocrack_y]
 #   type = DirichletBC
 #   variable = disp_y
 #   boundary = 600
 #   value = 0.0
 # [../]

# [./no_x]
#   type = DirichletBC
#   variable = disp_x
#   boundary = 500
#   value = 0.0
# [../]
# [./no_y]
#   type = DirichletBC
#   variable = disp_y
#   boundary = 500
#   value = 0.0
# [../]
  [./no_z]
    type = DirichletBC
    variable = disp_z
    boundary = 500
    value = 0.0
  [../]

  [./no_x]
    type = DirichletBC
    variable = disp_x
    boundary = 700
    value = 0.0
  [../]

  [./Pressure]
    [./Side1]
      boundary = 400
      function = rampConstant
    [../]
  [../]

[] # BCs

[Materials]
  [./stiffStuff]
    type = Elastic
    block = 1

    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z

    youngs_modulus = 207000
    poissons_ratio = 0.3
    thermal_expansion = 1e-5
    compute_JIntegral = true
  [../]

  [./density]
    type = Density
    block = '1'
    density = 8000.0
  [../]
[]


[Executioner]

   type = Transient
  # Two sets of linesearch options are for petsc 3.1 and 3.3 respectively

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'


  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101'


  line_search = 'none'

   l_max_its = 50
   nl_max_its = 20
   nl_abs_tol = 1e-5
   l_tol = 1e-2

   start_time = 0.0
   dt = 1

   end_time = 1
   num_steps = 1

[]

[Postprocessors]
  [./_dt]
    type = TimestepSize
  [../]

  [./nl_its]
    type = NumNonlinearIterations
  [../]

  [./lin_its]
    type = NumLinearIterations
  [../]
  [./J1]
    type = JIntegral
    q = q1
    crack_front_definition = crackFrontDefinition
  [../]
  [./J1_large]
    type = JIntegral
    q = q1
    crack_front_definition = crackFrontDefinition
    large = true
  [../]
  [./J2]
    type = JIntegral
    q = q2
    crack_front_definition = crackFrontDefinition
  [../]
   [./J2_large]
    type = JIntegral
    q = q2
    crack_front_definition = crackFrontDefinition
    large = true
  [../]
 [./J3]
    type = JIntegral
    q = q3
    crack_front_definition = crackFrontDefinition
  [../]
  [./J3_large]
    type = JIntegral
    q = q3
    crack_front_definition = crackFrontDefinition
    large = true
  [../]
  [./J4]
    type = JIntegral
    q = q4
    crack_front_definition = crackFrontDefinition
  [../]
  [./J4_large]
    type = JIntegral
    q = q4
    crack_front_definition = crackFrontDefinition
    large = true
  [../]
  [./J5]
    type = JIntegral
    q = q5
    crack_front_definition = crackFrontDefinition
  [../]
  [./J5_large]
    type = JIntegral
    q = q5
    crack_front_definition = crackFrontDefinition
    large = true
  [../]
[]


[Output]
  linear_residuals = true
  file_base = j_integral_test_out
  interval = 1
  output_initial = true
  exodus = true
[]
