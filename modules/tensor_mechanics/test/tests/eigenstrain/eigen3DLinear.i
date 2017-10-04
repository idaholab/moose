#
# This test exercises the smoothing of eigenstrains.
#
# In the x direction, the eigenstrain is constant (-1e-6).
# In the y direction, the eigenstrain is linear (-1e-6*y).
# In the z direction, the eigenstrain is quadratic (-1e-6*z*z).
#
# Young's modulus is 1e8. Poisson's ratio is zero.
#
# All nodes are restrained in all directions.
#
# The mesh is two elements.  The first element is a cube with one corner
# at (0,0,0) and the other at (1,1,1).  The second element is a cube with
# one corner at (1,1,1) and the other at (3,3,3).  The elements are linear.
#
# The eigenstrain smoothing forces all eigenstrains to be constant in an
# element for linear elements using volume averaging.  Thus, the eigenstrain
# in the x direction is -1e-6.  In the y direction, integrating -1e-6*x from
# 0 to 1 and dividing by length (1) gives -5e-7.  Integrating from 1 to 3 and
# dividing by length (2) gives -2e-6.  In the z direction, integrating
# -1e-6*z*z from 0 to 1 and dividing by length (a) gives -3.33e-7.  Integrating
# from 1 to 3 and dividing by length (2) gives -4.33e-6.
#
#              Element #1   Element #2
# --------------------------------------
#  eigen_xx     -1e-6        -1e-6
#  eigen_yy     -5e-7        -2e-6
#  eigen_zz     -3.33e-7     -4.33e-6
#  stress_xx     100          100
#  stress_yy      50          200
#  stress_zz      33.33       433.33
#

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  file = eigen3DLinear.e
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

[AuxKernels]
  [./stress_xx]
    type = RankTwoAux
    index_i = 0
    index_j = 0
    variable = stress_xx
    rank_two_tensor = stress
  [../]
  [./stress_yy]
    type = RankTwoAux
    index_i = 1
    index_j = 1
    variable = stress_yy
    rank_two_tensor = stress
  [../]
  [./stress_zz]
    type = RankTwoAux
    index_i = 2
    index_j = 2
    variable = stress_zz
    rank_two_tensor = stress
  [../]
[]

[Functions]
  [./constant_x]
    type = ParsedFunction
    value = '-1e-6'
  [../]
  [./linear_y]
    type = ParsedFunction
    value = '-1e-6*y'
  [../]
  [./quadratic_z]
    type = ParsedFunction
    value = '-1e-6*z*z'
  [../]
[]

[BCs]
  [./fixed_x]
    type = PresetBC
    variable = disp_x
    value = 0
    boundary = 'left right'
  [../]
  [./fixed_y]
    type = PresetBC
    variable = disp_y
    value = 0
    boundary = 'bottom top'
  [../]
  [./disp_z]
    type = PresetBC
    variable = disp_z
    value = 0
    boundary = 'back front'
  [../]
[]

[Kernels]
  [./TensorMechanics]
    use_displaced_mesh = true
  [../]
[]

[Materials]
  [./et]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e8
    poissons_ratio = 0.0
  [../]
  [./strn]
    type = ComputeIncrementalSmallStrain
    eigenstrain_names = 'eigen_x eigen_y eigen_z'
  [../]
  [./stress]
    type = ComputeFiniteStrainElasticStress
  [../]
  [./eigen_x]
    type = ComputeEigenstrain
    prefactor = mat_x
    eigen_base = '1.0 0.0 0.0 0.0 0.0 0.0'
    eigenstrain_name = 'eigen_x'
  [../]
  [./eigen_y]
    type = ComputeEigenstrain
    prefactor = mat_y
    eigen_base = '0.0 1.0 0.0 0.0 0.0 0.0'
    eigenstrain_name = 'eigen_y'
  [../]
  [./eigen_z]
    type = ComputeEigenstrain
    prefactor = mat_z
    eigen_base = '0.0 0.0 1.0 0.0 0.0 0.0'
    eigenstrain_name = 'eigen_z'
  [../]
  [./mat_x]
    type = GenericFunctionMaterial
    prop_values = constant_x
    prop_names = mat_x
  [../]
  [./mat_y]
    type = GenericFunctionMaterial
    prop_values = linear_y
    prop_names = mat_y
  [../]
  [./mat_z]
    type = GenericFunctionMaterial
    prop_values = quadratic_z
    prop_names = mat_z
  [../]
[]

[Executioner]
  type = Transient
  dt = 1.0
  end_time = 1.0
  solve_type = PJFNK
[]

[Outputs]
  exodus = true
  console = true
[]
