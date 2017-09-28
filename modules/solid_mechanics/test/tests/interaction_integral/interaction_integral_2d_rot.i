#This tests the J-Integral evaluation capability.
#This is a 2d plane strain model
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
  displacements = 'disp_x disp_y'
[]

[Mesh]
  file = crack2d_rot.e
[]


[Variables]
  [./disp_x]
  [../]
  [./disp_y]
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
  boundary = 800
  crack_direction_method = CrackDirectionVector
  crack_direction_vector = '0 1 0'
  2d = true
  axis_2d = 2
  radius_inner = '4.0 4.5 5.0 5.5 6.0'
  radius_outer = '4.5 5.0 5.5 6.0 6.5'
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

  [./crack_y]
    type = DirichletBC
    variable = disp_x
    boundary = 100
    value = 0.0
  [../]

  [./no_x]
    type = DirichletBC
    variable = disp_y
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

    youngs_modulus = 207000
    poissons_ratio = 0.3
    thermal_expansion = 1e-5
    formulation = NonlinearPlaneStrain
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
   nl_abs_tol = 1e-5
   l_tol = 1e-2

   start_time = 0.0
   dt = 1

   end_time = 1
   num_steps = 1

[]

[Outputs]
  file_base = interaction_integral_2d_rot_out
  exodus = true
  csv = true
[]
