#This tests the J-Integral evaluation capability.
#This is a 3d extrusion of a 2d plane strain model with 2 elements
#through the thickness, and calculates the J-Integrals using options
#to treat it as 3d.
#The analytic solution for J1 is 2.434.  This model
#converges to that solution with a refined mesh.
#Reference: National Agency for Finite Element Methods and Standards (U.K.):
#Test 1.1 from NAFEMS publication "Test Cases in Linear Elastic Fracture
#Mechanics" R0020.



[GlobalParams]
  order = FIRST
#  order = SECOND
  family = LAGRANGE
  disp_x = disp_x
  disp_y = disp_y
  disp_z = disp_z
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  file = crack3d.e
#  partitioner = centroid
#  centroid_partitioner_direction = z
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

[DomainIntegral]
  integrals = 'InteractionIntegralKI InteractionIntegralKII InteractionIntegralKIII'
  crack_front_points = '0 -10 .5
                        0 -10 0
                        0 -10 -.5'
  closed_loop = false # if user provides 'crack_front_points' instead of 'boundary', 'closed_loop' should be set by user!
  crack_direction_method = CrackDirectionVector
  crack_direction_vector = '1 0 0'
  radius_inner = '4.0 5.5'
  radius_outer = '5.5 7.0'
  block = 1
  youngs_modulus = 207000
  poissons_ratio = 0.3
  solid_mechanics = true
[]

[SolidMechanics]
  [./solid]
  [../]
[]

[AuxKernels]
  [./stress_xx]               # computes stress components for output
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xx
    index = 0
    execute_on = timestep_end     # for efficiency, only compute at the end of a timestep
  [../]
  [./stress_yy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yy
    index = 1
    execute_on = timestep_end
  [../]
  [./stress_zz]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_zz
    index = 2
    execute_on = timestep_end
  [../]
  [./vonmises]
    type = MaterialTensorAux
    tensor = stress
    variable = vonmises
    quantity = vonmises
    execute_on = timestep_end
  [../]
  [./SED]
    type = MaterialRealAux
    variable = SED
    property = strain_energy_density
    execute_on = timestep_end
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
  [./no_z2]
    type = DirichletBC
    variable = disp_z
    boundary = 510
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
   nl_abs_tol = 1e-7
   l_tol = 1e-3

   start_time = 0.0
   dt = 1

   end_time = 1
   num_steps = 1

[]

[Outputs]
  file_base = interaction_integral_3d_points_out
  exodus = true
  csv = true
[]
