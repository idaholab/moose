# T-stress test for a through crack in a wide ("infinite") plate.
# For a finer mesh this problem converges to the solution T = -sigma.
# Ref: T.L. Anderson, Fracture Mechanics: Fundamentals and Applications

[GlobalParams]
  order = FIRST
  family = LAGRANGE
  disp_x = disp_x
  disp_y = disp_y
[]

[Mesh]
  file = crack_infinite_plate.e
  displacements = 'disp_x disp_y'
  partitioner = centroid
  centroid_partitioner_direction = z
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
  [./rampConstantUp]
    type = PiecewiseLinear
    x = '0. 1.'
    y = '0. 1.'
    scale_factor = -100
  [../]
[]

[DomainIntegral]
  integrals = 'JIntegral InteractionIntegralKI InteractionIntegralT'
  boundary = 1001
  crack_direction_method = CrackDirectionVector
  crack_direction_vector = '1 0 0'
  radius_inner = '0.06 0.08 0.10'
  radius_outer = '0.08 0.10 0.12'
  block = 1
  youngs_modulus = 30e+6
  poissons_ratio = 0.3
  2d = true
  axis_2d = 2
  symmetry_plane = 1
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

  [./no_x]
    type = DirichletBC
    variable = disp_x
    boundary = 300
    value = 0.0
  [../]

  [./no_y]
    type = DirichletBC
    variable = disp_y
    boundary = 100
    value = 0.0
  [../]

  [./Pressure]
    [./top]
      boundary = 200
      function = rampConstantUp
    [../]
  [../]
[] # BCs

[Materials]
  [./stiffStuff]
    type = Elastic
    block = 1

    disp_x = disp_x
    disp_y = disp_y

    youngs_modulus = 30e+6
    poissons_ratio = 0.3
    compute_JIntegral = true
  [../]
[]


[Executioner]

   type = Transient
  # Two sets of linesearch options are for petsc 3.1 and 3.3 respectively

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'


#  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      4'


  line_search = 'none'

   l_max_its = 50
   nl_max_its = 20
   nl_abs_tol = 1e-9
   nl_rel_tol = 1e-12
   l_tol = 1e-2

   start_time = 0.0
   dt = 1

   end_time = 1
   num_steps = 1

[]

[Outputs]
  file_base = t_stress_crack_infinite_plate_out
  exodus = true
[]
