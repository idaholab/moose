#
# Hoop stress
#
# This test checks that hoop stress is calculated correctly for the default orientation.
# It calculates the hoop stress for a hoop cenetered at (-25,0,0) with the default vector (0,1,0).
# The hoop has a radius = 20, t = 1.
#
# Hoop stress should be P*r/t -> 1e3*20/1 = 20e3
#
# The output hoop stress is close to this value (nonlinear geometry is on) for all
# elements.
#

[GlobalParams]
  disp_x = disp_x
  disp_y = disp_y
  disp_z = disp_z
[]

[Mesh]#Comment
  file = hoop_default_yaxis.e
  displacements = 'disp_x disp_y disp_z'
[] # Mesh

[Functions]
  [./pressure]
    type = PiecewiseLinear
    x = '0. 1.'
    y = '0. 1.'
    scale_factor = 1e3
  [../]
[] # Functions

[Variables]

  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]

  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]

  [./disp_z]
    order = FIRST
    family = LAGRANGE
  [../]

[] # Variables

[AuxVariables]

  [./stress_xx]
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
  [./stress_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_zx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./hoop2]
    order = CONSTANT
    family = MONOMIAL
    block = 2
  [../]
  [./radial2]
    order = CONSTANT
    family = MONOMIAL
    block = 2
  [../]
  [./axial2]
    order = CONSTANT
    family = MONOMIAL
    block = 2
  [../]

[] # AuxVariables

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  [../]
[]

[AuxKernels]

  [./stress_xx]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xx
    index = 0
    execute_on = timestep_end
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
  [./stress_xy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xy
    index = 3
    execute_on = timestep_end
  [../]
  [./stress_yz]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yz
    index = 4
    execute_on = timestep_end
  [../]
  [./stress_zx]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_zx
    index = 5
    execute_on = timestep_end
  [../]
  [./hoop2]
    type = MaterialTensorAux
    tensor = stress
    quantity = hoop
    variable = hoop2
    block = 2
    execute_on = timestep_end
  [../]
  [./radial2]
    type = MaterialTensorAux
    tensor = stress
    quantity = radial
    variable = radial2
    block = 2
    execute_on = timestep_end
  [../]
  [./axial2]
    type = MaterialTensorAux
    tensor = stress
    quantity = axial
    variable = axial2
    block = 2
    execute_on = timestep_end
  [../]

[] # AuxKernels

[BCs]

  [./fix_x]
    type = DirichletBC
    variable = disp_x
    boundary = '11'
    value = 0
  [../]
  [./fix_y]
    type = DirichletBC
    variable = disp_y
    boundary = '200'
    value = 0
  [../]
  [./fix_z]
    type = DirichletBC
    variable = disp_z
    boundary = '13'
    value = 0
  [../]

  [./Pressure]
    [./internal_pressure]
      boundary = 1
      function = pressure
    [../]
  [../]

[] # BCs

[Materials]

  [./stiffStuff1]
    type = Elastic
    block = '2'

    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z

    youngs_modulus = 1e6
    poissons_ratio = 0.35
  [../]

[] # Materials

[Executioner]

  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'


  petsc_options = '-ksp_gmres_modifiedgramschmidt'
  petsc_options_iname = '-ksp_gmres_restart -pc_type  -pc_hypre_type'
  petsc_options_value = '201                 hypre     boomeramg     '


  line_search = 'none'


  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-7

  l_max_its = 20

  start_time = 0.0
  dt = 1.0
  num_steps = 1
  end_time = 1.0
[] # Executioner

[Outputs]
  output_initial = true
  exodus = true
  print_linear_residuals = true
  print_perf_log = true
[] # Outputs
