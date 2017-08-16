# Creep test using Abaqus CREEP subroutine interface

[Mesh]
  # file = cantilever.e
  type = GeneratedMesh
  dim = 3
  xmin = 0.0
  xmax = 15.0
  ymin = 0.0
  ymax = 1.5
  zmin = 0.0
  zmax = 3.0
  nx = 20
  ny = 2
  nz = 4
  displacements = 'disp_x disp_y disp_z'
[]

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
[]

[AuxVariables]
  active = ''
  [./stress_mag]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xx]
    family = MONOMIAL
  [../]
  [./stress_yy]
    family = MONOMIAL
  [../]
  [./stress_zz]
    family = MONOMIAL
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  [../]
[]

[AuxKernels]
  active = ''
  [./stress_yy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yy
    index = 1
    point1 = '0 0 0'
    point2 = '0 1 0'
  [../]
  [./stress_mag]
    type = MaterialTensorAux
    variable = stress_mag
    index = 1
    tensor = stress
    point1 = '0 0 0'
    point2 = '0 1 0'
    quantity = vonmises
  [../]
  [./stress_xx]
    type = MaterialTensorAux
    variable = stress_xx
    index = 0
    tensor = stress
    point1 = '0 0 0'
    point2 = '0 1 0'
    block = 1
  [../]
  [./stress_zz]
    type = MaterialTensorAux
    variable = stress_zz
    index = 2
    tensor = stress
    point1 = '0 0 0'
    point2 = '0 1 0'
  [../]
[]

[BCs]
  [./bottom_x]
    type = DirichletBC
    variable = disp_x
    boundary = right
    value = 0.0
  [../]
  [./bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = right
    value = 0.0
  [../]
  [./bottom_z]
    type = DirichletBC
    variable = disp_z
    boundary = right
    value = 0.0
  [../]
  [./Pressure]
    [./press]
      disp_z = disp_z
      disp_y = disp_y
      disp_x = disp_x
      factor = -5
      boundary = left
    [../]
  [../]
[]

[Materials]
  [./solid]
    type = Elastic
    block = 0
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    youngs_modulus = 209000
    poissons_ratio = 0.3
  [../]
  [./creep]
    type = AbaqusCreepMaterial
    block = 0
    plugin = ../../plugins/creep_usr_creep
    poissons_ratio = 0.3
    integration_flag = 1
    num_state_vars = 2
    youngs_modulus = 209000
    solve_definition = 5
  [../]
[]

[Executioner]
  # l_max_its = 60
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'


  petsc_options = '-snes_ksp_ew '
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type'
  petsc_options_value = '101 hypre boomeramg'
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-7
  l_tol = 1e-5
  start_time = 0.0
  dt = 1
  num_steps = 1
[]

[Outputs]
  file_base = out
  [./exodus]
    type = Exodus
    elemental_as_nodal = true
  [../]
[]

