# The purpose of this test is to verify that the procedures for initializing
# the solution on nodes/elements affected by XFEM works correctly in both
# serial and parallel.

# The crack cuts near to domain boundaries in parallel, and the displacement
# solution will be wrong in parallel if this is not done correctly.  This
# test also has multiple aux variables of various types that are only computed
# on initialization, and which will be wrong if the XFEM initializtion
# is not done correctly.

[GlobalParams]
  order = FIRST
  family = LAGRANGE
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = true
[]

[XFEM]
  output_cut_plane = true
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 7
  ny = 7
  xmin = 0.0
  xmax = 25.0
  ymin = -12.5
  ymax = 12.5
  elem_type = QUAD4
[]

[UserObjects]
  [./line_seg_cut_set_uo]
    type = LineSegmentCutSetUserObject
    cut_data ='0.0000e+000  0.0000e+000  5.5000e+000  0.0000e+000  0.0   0.0
               5.5000e+000  0.0000e+000  2.5500e+001  0.0000e+000  0.05  1.05'
  [../]
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[AuxVariables]
  [./const_monomial]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./first_monomial]
    order = FIRST
    family = MONOMIAL
  [../]
  [./first_linear]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Physics/SolidMechanics/QuasiStatic]
  [./all]
    strain = FINITE
    add_variables = true
    planar_formulation = PLANE_STRAIN
  [../]
[]

[AuxKernels]
  [./const_monomial]
    type = FunctionAux
    function = 'dummy'
    variable = const_monomial
    execute_on = 'initial'
  [../]
  [./first_monomial]
    type = FunctionAux
    function = 'dummy'
    variable = first_monomial
    execute_on = 'initial'
  [../]
  [./first_linear]
    type = FunctionAux
    function = 'dummy'
    variable = first_linear
    execute_on = 'initial'
  [../]
[]

[Functions]
  [./dummy]
    type = ParsedFunction
    expression = 'x*x+y*y'
  [../]
  [./disp_top_y]
    type = PiecewiseLinear
    x = '0 1'
    y = '0 0.1'
  [../]
[]

[BCs]
  [./top_y]
    type = FunctionDirichletBC
    boundary = 2
    variable = disp_y
    function = disp_top_y
  [../]

  [./bottom_y]
    type = DirichletBC
    boundary = 0
    variable = disp_y
    value = 0.0
  [../]

  [./right_x]
    type = DirichletBC
    boundary = 1
    variable = disp_x
    value = 0.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  [../]
  [./stress]
    type = ComputeFiniteStrainElasticStress
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      8'

  petsc_options = '-snes_ksp_ew'

  l_max_its = 100
  nl_max_its = 25
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-8

  start_time = 0.0
  dt = 0.1
  end_time = 1.0
  max_xfem_update = 1
[]


[Outputs]
  exodus = true
  [./console]
    type = Console
    output_linear = true
  [../]
[]
