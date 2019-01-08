#  Constant mass in RZ using Tensor Mechanics
#
# This test forces an RZ mesh to move through a series of displacements
#   in order to test whether the mass is constant.  The density is chosen
#   such that the mass is 2.5.
# This test is a duplicate of the rz.i test for solid mechanics, and the
#   output of this tensor mechanics test is compared to the original
#   solid mechanics output.  The duplication is necessary to test the
#   migrated tensor mechanics version while maintaining tests for solid mechanics.

[Mesh]
  file = elastic_patch_rz.e
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Problem]
  coord_type = RZ
[]

[Functions]
  [./x101]
    type = PiecewiseLinear
    x = '0 5 6'
    y = '0 0 0.24'
  [../]
  [./y101]
    type = PiecewiseLinear
    x = '0 6'
    y = '0 0'
  [../]
  [./x102]
    type = PiecewiseLinear
    x = '0 4 5'
    y = '0 0 0.24'
  [../]
  [./y102]
    type = PiecewiseLinear
    x = '0 1 2 3'
    y = '0 0 0.12 0'
  [../]
  [./x103]
    type = PiecewiseLinear
    x = '0 4 5'
    y = '0 0 0.24'
  [../]
  [./y103]
    type = PiecewiseLinear
    x = '0 1    3    4'
    y = '0 0.12 0.12 0'
  [../]
  [./x104]
    type = PiecewiseLinear
    x = '0 5 6'
    y = '0 0 0.24'
  [../]
  [./y104]
    type = PiecewiseLinear
    x = '0 2 3    4'
    y = '0 0 0.12 0'
  [../]
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

[Kernels]
  [./TensorMechanics]
    use_displaced_mesh = true
  [../]
[]

[BCs]

  [./101x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 101
    function = x101
  [../]
  [./101y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 101
    function = y101
  [../]

  [./102x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 102
    function = x102
  [../]
  [./102y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 102
    function = y102
  [../]

  [./103x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 103
    function = x103
  [../]
  [./103y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 103
    function = y103
  [../]

  [./104x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 104
    function = x104
  [../]
  [./104y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 104
    function = y104
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    block = PATCH
    youngs_modulus = 1e6
    poissons_ratio = 0.0
  [../]

  [./small_strain_rz]
    type = ComputeAxisymmetricRZSmallStrain
    block = PATCH
  [../]

  [./elastic_stress]
    type = ComputeLinearElasticStress
    block = PATCH
  [../]
[]

[Executioner]
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  # Two sets of linesearch options are for petsc 3.1 and 3.3 respectively
  petsc_options_iname = '-pc_type -ksp_gmres_restart'
  petsc_options_value = 'lu       101'

  line_search = 'none'
  nl_abs_tol = 1e-10
  l_max_its = 20

  start_time = 0.0
  dt = 1
  num_steps = 6
  end_time = 6.0
[]

[Outputs]
  [./out]
    type = Exodus
    elemental_as_nodal = true
    file_base = rz_out
  [../]
[]

[Postprocessors]
  [./mass]
    type = Mass
    variable = disp_x
    execute_on = 'initial timestep_end'
  [../]
[]
