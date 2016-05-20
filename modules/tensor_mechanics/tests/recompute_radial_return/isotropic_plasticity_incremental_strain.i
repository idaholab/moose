# This simulation uses the piece-wise linear strain hardening model
# with the incremental small strain formulation; incremental small strain
# is required to produce the strain_increment for the DiscreteRadialReturnStressIncrement
# class, which handles the calculation of the stress increment to return
# to the yield surface in a J2 (isotropic) plasticity problem.
#
#  This test assumes a Poissons ratio of zero and applies a displacement loading
# condition on the top in the y direction while fixing the displacement in the x
# and z directions; thus, only the normal stress and the normal strains in the
# y direction are compared in this problem.
#
# A similar problem was run in Abaqus on a similar 1 element mesh and was used
# to verify the SolidMechanics solution; this TensorMechanics code matches the
# SolidMechanics solution.
#
# Mechanical strain is the sum of the elastic and plastic strains but is different
# from total strain in cases with eigen strains, e.g. thermal strain.

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  order = FIRST
  family = LAGRANGE
[]

[Variables]
  [./disp_x]
  [../]

  [./disp_y]
  [../]

  [./disp_z]
  [../]
[]


[AuxVariables]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./plastic_strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./plastic_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./plastic_strain_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]

[]


[Functions]
  [./top_pull]
    type = ParsedFunction
    value = t*(0.01)
  [../]
  [./hf]
    type = PiecewiseLinear
    x = '0  0.00004 0.0001  0.1'
    y = '50   54    56       60'
  [../]
[]

[Kernels]
  [./TensorMechanics]
    use_displaced_mesh = true
  [../]
[]


[AuxKernels]
  [./stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
  [../]

  [./plastic_strain_yy]
    type = RankTwoAux
    rank_two_tensor = plastic_strain
    variable = plastic_strain_yy
    index_i = 1
    index_j = 1
  [../]

  [./plastic_strain_xx]
    type = RankTwoAux
    rank_two_tensor = plastic_strain
    variable = plastic_strain_xx
    index_i = 0
    index_j = 0
  [../]

  [./plastic_strain_zz]
    type = RankTwoAux
    rank_two_tensor = plastic_strain
    variable = plastic_strain_zz
    index_i = 2
    index_j = 2
  [../]
 []


[BCs]
  [./y_pull_function]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = top
    function = top_pull
  [../]

  [./x_sides]
    type = DirichletBC
    variable = disp_x
    boundary = 'left right'
    value = 0.0
  [../]

  [./y_bot]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]

  [./z_sides]
    type = DirichletBC
    variable = disp_z
    boundary = 'back front'
    value = 0.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    block = 0
    youngs_modulus = 2.5e5
    poissons_ratio = 0.0
  [../]
  [./small_strain]
    type = ComputeIncrementalSmallStrain
    block = 0
  [../]


  [./isotropic_plasticity_recompute]
    type = RecomputeRadialReturnIsotropicPlasticity
    block = 0
    yield_stress = 25.
    hardening_constant = 1000.
    relative_tolerance = 1e-10
    absolute_tolerance = 1e-12
    max_iterations = 50
    # output_iteration_info_on_error = true
  [../]

  [./radial_return_stress]
    type = ComputeReturnMappingStress
    block = 0
    return_mapping_models = 'isotropic_plasticity_recompute'
  [../]
[]

[Executioner]
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101'

  line_search = 'none'

  l_max_its = 20
  nl_max_its = 20
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12
  l_tol = 1e-9

  start_time = 0.0
  end_time = 0.01875
  dt = 0.00125
  dtmin = 0.0001
[]


[Outputs]
  exodus = true
  print_linear_residuals = true
  print_perf_log = true
[]
