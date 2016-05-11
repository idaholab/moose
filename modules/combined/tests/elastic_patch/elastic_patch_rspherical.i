#
# Patch test for 1D spherical elements
#
# The 1D mesh is pinned at x=0.  The displacement at the outer node is set to
#   3e-3*X where X is the x-coordinate of that node.  That gives a strain of
#   3e-3 for the x, y, and z directions.
#
# Young's modulus is 1e6, and Poisson's ratio is 0.25.  This gives:
#
# Stress xx, yy, zz = E/(1+nu)/(1-2nu)*strain*((1-nu) + nu + nu) = 6000
#

[Problem]
  coord_type = RSPHERICAL
[]

[Mesh]#Comment
  file = elastic_patch_rspherical.e
  displacements = 'disp_x'
[] # Mesh

[Functions]
  [./ur]
    type = ParsedFunction
    value = '3e-3*x'
  [../]
[] # Functions

[Variables]

  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]

  [./temp]
    order = FIRST
    family = LAGRANGE
    initial_condition = 100.0
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
  [./density]
    order = CONSTANT
    family = MONOMIAL
  [../]

[] # AuxVariables

[SolidMechanics]
  [./solid]
    disp_r = disp_x
    temp = temp
  [../]
[]

[Kernels]

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
  [./density]
    type = MaterialRealAux
    property = density
    variable = density
  [../]

[] # AuxKernels

[BCs]

  [./ur]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = '1 2'
    function = ur
  [../]

  [./temp]
    type = DirichletBC
    variable = temp
    boundary = 1
    value = 117.56
  [../]

[] # BCs

[Materials]

  [./stiffStuff1]
    type = Elastic
    block = '1 2 3'

    disp_r = disp_x

    youngs_modulus = 1e6
    poissons_ratio = 0.25

    temp = temp
  [../]

  [./heat]
    type = HeatConductionMaterial
    block = '1 2 3'

    specific_heat = 0.116
    thermal_conductivity = 4.85e-4
  [../]

  [./density]
    type = Density
    block = '1 2 3'
    density = 0.283
    disp_r = disp_x
  [../]

[] # Materials

[Executioner]

  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'



  petsc_options_iname = '-pc_type -ksp_gmres_restart'
  petsc_options_value = 'lu       101'

  line_search = 'none'


  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-11


  l_max_its = 20

  start_time = 0.0
  dt = 1.0
  num_steps = 1
  end_time = 1.0
[] # Executioner

[Outputs]
  exodus = true
[] # Outputs
