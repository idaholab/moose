#
# This problem is taken from the Abaqus verification manual:
#   "1.5.4 Patch test for axisymmetric elements"
# The stress solution is given as:
#   xx = yy = zz = 2000
#   xy = 400
#

[Problem]
  coord_type = RZ
[]

[Mesh]#Comment
  file = elastic_patch_rz.e
#  displacements = 'disp_x disp_y'
[] # Mesh

[Functions]
  [./ur]
    type = ParsedFunction
    value = '1e-3*x'
  [../]
  [./uz]
    type = ParsedFunction
    value = '1e-3*(x+y)'
  [../]
  [./body]
    type = ParsedFunction
    value = '-400/x'
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

  [./temp]
    order = FIRST
    family = LAGRANGE
    initial_condition = 117.56
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

[] # AuxVariables

[SolidMechanics]
  [./solid]
    disp_r = disp_x
    disp_z = disp_y
  [../]
[]

[Kernels]

  [./body]
    type = BodyForce
    variable = disp_y
    value = 1
    function = body
  [../]

  [./heat]
    type = HeatConduction
    variable = temp
  [../]

[] # Kernels

[AuxKernels]

  [./stress_xx]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xx
    index = 0
  [../]
  [./stress_yy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yy
    index = 1
  [../]
  [./stress_zz]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_zz
    index = 2
  [../]
  [./stress_xy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xy
    index = 3
  [../]
  [./stress_yz]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yz
    index = 4
  [../]
  [./stress_zx]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_zx
    index = 5
  [../]

[] # AuxKernels

[BCs]

  [./ur]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 10
    function = ur
  [../]
  [./uz]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 10
    function = uz
  [../]

  [./temp]
    type = DirichletBC
    variable = temp
    boundary = 10
    value = 117.56
  [../]

[] # BCs

[Materials]

  [./stiffStuff1]
    type = Elastic
    block = 1

    disp_r = disp_x
    disp_z = disp_y

    youngs_modulus = 1e6
    poissons_ratio = 0.25

    temp = temp
  [../]

  [./heat]
    type = HeatConductionMaterial
    block = 1

    specific_heat = 0.116
    thermal_conductivity = 4.85e-4
  [../]

  [./density]
    type = Density
    block = 1
    density = 0.283
    disp_r = disp_x
    disp_z = disp_y
  [../]

[] # Materials

[Executioner]

  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'



  petsc_options_iname = '-pc_type -ksp_gmres_restart'
  petsc_options_value = 'lu       101'


  line_search = 'none'


  nl_abs_tol = 1e-11
  nl_rel_tol = 1e-12


  l_max_its = 20

  start_time = 0.0
  dt = 1.0
  num_steps = 1
  end_time = 1.0
[] # Executioner

[Outputs]
  [./out]
    type = Exodus
    elemental_as_nodal = true
  [../]
[] # Outputs
