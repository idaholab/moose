#
# Rotation Test
#
# This test is designed to compute a uniaxial stress and then follow that
# stress as the mesh is rotated 90 degrees.
#
# The mesh is composed of one block with a single element.  The nodal
# displacements in the x and y directions are prescribed.  Poisson's
# ratio is zero.
#

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    elem_type = HEX8
    displacements = 'ux uy uz'
  []
  [./side1n1]
    input = gen
    type = ExtraNodesetGenerator
    coord = '0.0 0.0 0.0'
    boundary = 6
  [../]
  [./side1n2]
    input = side1n1
    type = ExtraNodesetGenerator
    coord = '1.0 0.0 0.0'
    boundary = 7
  [../]
  [./side2n1]
    input = side1n2
    type = ExtraNodesetGenerator
    coord = '0.0 1.0 0.0'
    boundary = 8
  [../]
  [./side2n2]
    input = side2n1
    type = ExtraNodesetGenerator
    coord = '1.0 1.0 0.0'
    boundary = 9
  [../]
  [./side3n1]
    input = side2n2
    type = ExtraNodesetGenerator
    coord = '0.0 1.0 1.0'
    boundary = 10
  [../]
  [./side3n2]
    input = side3n1
    type = ExtraNodesetGenerator
    coord = '1.0 1.0 1.0'
    boundary = 11
  [../]
  [./side4n1]
    input = side3n2
    type = ExtraNodesetGenerator
    coord = '0.0 0.0 1.0'
    boundary = 12
  [../]
  [./side4n2]
    input = side4n1
    type = ExtraNodesetGenerator
    coord = '1.0 0.0 1.0'
    boundary = 13
  [../]
[]

[Variables]
  [./ux]
    block = 0
  [../]
  [./uy]
    block = 0
  [../]
  [./uz]
    block = 0
  [../]
[]

[Functions]
  [./side2uxfunc]
    type = ParsedFunction
    expression = cos(pi/2*t)-1
  [../]
  [./side2uyfunc]
    type = ParsedFunction
    expression = sin(pi/2*t)
  [../]
  [./side3uxfunc]
    type = ParsedFunction
    expression = cos(pi/2*t)-sin(pi/2*t)-1
  [../]
  [./side3uyfunc]
    type = ParsedFunction
    expression = cos(pi/2*t)+sin(pi/2*t)-1
  [../]
  [./side4uxfunc]
    type = ParsedFunction
    expression = -sin(pi/2*t)
  [../]
  [./side4uyfunc]
    type = ParsedFunction
    expression = cos(pi/2*t)-1
  [../]
[]

[BCs]
  active = 'bcside1 bcside2ux bcside2uy bcside4ux bcside4uy bcside3uy bcside3ux bcx'
  [./bcside1]
    type = DirichletBC
    variable = 'uy uz'
    boundary = '6 7'
    value = 0
  [../]
  [./bcside2ux]
    type = FunctionDirichletBC
    variable = uy
    boundary = '8 9'
    function = side2uxfunc
  [../]
  [./bcside2uy]
    type = FunctionDirichletBC
    variable = uz
    boundary = '8 9'
    function = side2uyfunc
  [../]
  [./bcside3ux]
    type = FunctionDirichletBC
    variable = uy
    boundary = '10 11'
    function = side3uxfunc
  [../]
  [./bcside3uy]
    type = FunctionDirichletBC
    variable = uz
    boundary = '10 11'
    function = side3uyfunc
  [../]
  [./bcside4ux]
    type = FunctionDirichletBC
    variable = uy
    boundary = '12 13'
    function = side4uxfunc
  [../]
  [./bcside4uy]
    type = FunctionDirichletBC
    variable = uz
    boundary = '12 13'
    function = side4uyfunc
  [../]
  [./bot]
    type = DirichletBC
    variable = 'ux uy uz'
    boundary = back
    value = 0
  [../]
  [./topxz]
    type = DirichletBC
    variable = 'ux uz'
    boundary = front
    value = 0
  [../]
  [./topy]
    type = DirichletBC
    variable = uy
    boundary = front
    value = 1
  [../]
  [./bcx]
    type = DirichletBC
    variable = ux
    boundary = '6 7 8 9 10 11 12 13'
    value = 0
  [../]
[]

[Materials]
  [./crysp]
    type = FiniteStrainCrystalPlasticity
    block = 0
    disp_y = uy
    disp_x = ux
    slip_sys_file_name = input_slip_sys.txt
    disp_z = uz
    flowprops = ' 1 12 0.001 0.1'
    C_ijkl = '1.684e5 1.214e5 1.214e5 1.684e5 1.214e5 1.684e5 .754e5 .754e5 .754e5'
    nss = 12
    hprops = '1 541.5 60.8 109.8'
    gprops = '1 12 60.8'
    fill_method = symmetric9
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.01

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'



  petsc_options_iname = -pc_hypre_type
  petsc_options_value = boomerang
  dtmax = 0.01
  end_time = 1
  dtmin = 0.01
[]

[Outputs]
  file_base = rot_eg1
  solution_history = true
  [./exodus]
    type = Exodus
    use_displaced = true
  [../]
[]

[TensorMechanics]
  [./tensormech]
    disp_z = uz
    disp_y = uy
    disp_x = ux
  [../]
[]
