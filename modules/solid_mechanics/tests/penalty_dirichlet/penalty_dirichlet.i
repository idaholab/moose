#This tests the PenaltyDirichletBC and PenaltyFunctionDirichletBC on
#a simple solid mechanics problem.  A unit cube of material has
#penalty BCs on the left and right hand side.  The penalty for both of
#those BCs is the same as the stiffness of the block in that direction,
#so the compliance is due to the left BC, block, and right BC in equal
#portions. As a result, the displacement on the left side is 1/3 the
#displacement imposed by the function, and on the right side it is 2/3
#that value.

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0.0
  xmax = 1.0
  ymin = 0.0
  ymax = 1.0
  nx = 2
  ny = 2
  displacements = 'disp_x disp_y'
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
[]

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
[]

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
  [../]
[]

[AuxKernels]
  [./stress_xx]
    type = MaterialTensorAux
    variable = stress_xx
    tensor = stress
    index = 0
  [../]
  [./stress_yy]
    type = MaterialTensorAux
    variable = stress_yy
    tensor = stress
    index = 1
  [../]
  [./stress_zz]
    type = MaterialTensorAux
    variable = stress_zz
    tensor = stress
    index = 2
  [../]
[]

[Functions]
  [./pull]
    type = PiecewiseLinear
    xy_data = '0 0
               1 0.001'
  [../]
[]

[BCs]
  [./left_x]
    type = PenaltyDirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
    penalty = 1e4
  [../]
  [./right_x]
    type = FunctionPenaltyDirichletBC
    variable = disp_x
    boundary = right
    function = pull
    penalty = 1e4
  [../]
  [./bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]
[]

[Materials]
  [./solid]
    type = Elastic
    block = 0
    disp_x = disp_x
    disp_y = disp_y
    youngs_modulus = 1e4
    poissons_ratio = 0.0
  [../]
[]

[Executioner]
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew '
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type'
  petsc_options_value = '101 hypre boomeramg'
  nl_rel_tol = 1e-12
  l_tol = 1e-5
  start_time = 0.0
  dt = 1
  num_steps = 1
[]

[Outputs]
  file_base = out
  [./exodus]
    type = Exodus
  [../]
[]
