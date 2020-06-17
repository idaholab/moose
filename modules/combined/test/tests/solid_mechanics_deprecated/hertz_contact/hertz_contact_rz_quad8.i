# Hertz Contact: Sphere on sphere

# Spheres have the same radius, Young's modulus, and Poisson's ratio.

# Define E:
# 1/E = (1-nu1^2)/E1 + (1-nu2^2)/E2
#
# Effective radius R:
# 1/R = 1/R1 + 1/R2
#
# F is the applied compressive load.
#
# Area of contact a::
# a^3 = 3FR/4E
#
# Depth of indentation d:
# d = a^2/R
#
#
# Let R1 = R2 = 2.  Then R = 1.
#
# Let nu1 = nu2 = 0.25, E1 = E2 = 1.40625e7.  Then E = 7.5e6.
#
# Let F = 10000.  Then a = 0.1, d = 0.01.
#

[Problem]
  coord_type = RZ
[]

[Mesh]#Comment
  file = hertz_contact_rz_quad8.e
  displacements = 'disp_x disp_y'
[] # Mesh

[Functions]
  [./pressure]
    type = PiecewiseLinear
    x = '0. 1. 2.'
    y = '0. 1. 1.'
    scale_factor = 795.77471545947674 # 10000/pi/2^2
  [../]
  [./disp_y]
    type = PiecewiseLinear
    x = '0.  1.    2.'
    y = '0. -0.01 -0.01'
  [../]
[] # Functions

[Variables]

  [./disp_x]
    order = SECOND
    family = LAGRANGE
  [../]

  [./disp_y]
    order = SECOND
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
  [./vonmises]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./hydrostatic]
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
  [./vonmises]
    type = MaterialTensorAux
    tensor = stress
    variable = vonmises
    quantity = vonmises
  [../]
  [./hydrostatic]
    type = MaterialTensorAux
    tensor = stress
    variable = hydrostatic
    quantity = hydrostatic
  [../]

[] # AuxKernels

[BCs]

  [./base_y]
    type = DirichletBC
    variable = disp_y
    boundary = 1000
    value = 0.0
  [../]

  [./symm_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
  [./disp_y]
    type = FunctionDirichletBC
    variable = disp_y
    component = 1
    boundary = 2
    function = disp_y
  [../]

[] # BCs

[Contact]
  [./dummy_name]
    primary = 1000
    secondary = 100
    disp_x = disp_x
    disp_y = disp_y
    penalty = 1e7
  [../]
[]

[Materials]

  [./stiffStuff1]
    type = Elastic
    block = 1

    disp_r = disp_x
    disp_z = disp_y

    youngs_modulus = 1.40625e7
    poissons_ratio = 0.25
  [../]
  [./stiffStuff2] # Rigid block
    type = Elastic
    block = 1000

    disp_r = disp_x
    disp_z = disp_y

    youngs_modulus = 1e6
    poissons_ratio = 0.0
  [../]

[] # Materials

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  []
[]

[Executioner]

  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'
  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre    boomeramg      101'


  line_search = 'none'


  nl_abs_tol = 1e-7

  l_max_its = 200

  start_time = 0.0
  dt = 0.5
  end_time = 2.0

  [./Quadrature]
    order = THIRD
  [../]

[] # Executioner

[Postprocessors]
  [./maxdisp]
    type = NodalVariableValue
    nodeid = 103 # 104-1 where 104 is the exodus node number of the top-left node
    variable = disp_y
  [../]
[]

[Outputs]
  elemental_as_nodal = true
  [./out]
    type = Exodus
    elemental_as_nodal = true
  [../]
[] # Output
