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
# one corner at (1,1,1) and the other at (3,3,3).  The elements are quadratic
# (Hex20).
#
# The eigenstrain smoothing forces all eigenstrains to be linear in an
# element for quadratic elements.  The eigenstrain in the x direction is
# constant and does not change, -1e-6.  In the y direction, the eigenstrain is
# linear and does not change, -1e-6*y.  In the z direction, we find the linear
# fit by setting the derivative of the integral of the square of the error to
# zero:
# y = a*z*z where a=-1e-6, y' = m*z + b, e = a*z*z - m*z -b,
# e^2 = a^2*z^4 - 2*a*m*z^3 - 2*a*b*z^2 + m^2*z^2 + 2*b*m*z + b^2
# int(e^2) = a^2*z^5/5 - a*m*z^4/2 - 2*a*b*z^3/3 + m^2*z^3/3 + b*m*z^2 + b^2*z
# g = int(e^2,0,1) = a^2/5 - a*m/2 - 2*a*b/3 + m^2/3 + b*m + b^2
# dg/dm = -a/2 + 2*m/3 + b = 0
# dg/db = -2*a/3 + m + 2*b = 0
# which leads to m = a, b = -a/6 over (0,1).
#
# A similar procedure over (1,3) gives m = 4*a, b = -11*a/3.
#
# The stress reported by the code in a constant monomial is the volume-averaged
# stress.  The volume-averaged eigenstrain over (0,1) for the linear eigenstrain
# is a/2 and over (1,3) is 2*a.  The volume-averaged eigenstrain for the
# linearized quadratic eigenstrain over (0,1) is a/3 and over (1,3) is 13*a/3.
#
#              Analytic                  Computed
#              Element #1   Element #2   Element #1   Element #2
# ---------------------------------------------------------------
#  eigen_xx     -1e-6        -1e-6
#  eigen_yy     -5e-7        -2e-6
#  eigen_zz     -3.33e-7     -4.33e-6
#  stress_xx     100          100          100          100
#  stress_yy      50          200           50          200
#  stress_zz      33.33       433.33        35          440
#
# The linear fit of the quadratic field in the code is done using least squares
# and a system of normal equations.  Given the location of the integration
# points in the element, solving the least squares problem yields m = a,
# b = -3*a/20 for the first element and m = 4*a, b = -18*a/5 for the second.
#
# Thus, the linearized quadratic solution has an error.  However, the error
# will reduce with smaller elements, and the error is less than would be
# obtained by using the linear temperature field returned by linear interpolation
# from the corner nodes.
#

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  file = eigen3DQuadratic.e
[]

[Variables]
  [./disp_x]
    order = SECOND
  [../]
  [./disp_y]
    order = SECOND
  [../]
  [./disp_z]
    order = SECOND
  [../]
[]

[AuxVariables]
  [./sxx_constant]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./syy_constant]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./szz_constant]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./sxx_first]
    order = FIRST
    family = MONOMIAL
  [../]
  [./syy_first]
    order = FIRST
    family = MONOMIAL
  [../]
  [./szz_first]
    order = FIRST
    family = MONOMIAL
  [../]
  [./sxx_second]
    order = SECOND
    family = MONOMIAL
  [../]
  [./syy_second]
    order = SECOND
    family = MONOMIAL
  [../]
  [./szz_second]
    order = SECOND
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./sxx_constant]
    type = RankTwoAux
    index_i = 0
    index_j = 0
    variable = sxx_constant
    rank_two_tensor = stress
  [../]
  [./syy_constant]
    type = RankTwoAux
    index_i = 1
    index_j = 1
    variable = syy_constant
    rank_two_tensor = stress
  [../]
  [./szz_constant]
    type = RankTwoAux
    index_i = 2
    index_j = 2
    variable = szz_constant
    rank_two_tensor = stress
  [../]
  [./sxx_first]
    type = RankTwoAux
    index_i = 0
    index_j = 0
    variable = sxx_first
    rank_two_tensor = stress
  [../]
  [./syy_first]
    type = RankTwoAux
    index_i = 1
    index_j = 1
    variable = syy_first
    rank_two_tensor = stress
  [../]
  [./szz_first]
    type = RankTwoAux
    index_i = 2
    index_j = 2
    variable = szz_first
    rank_two_tensor = stress
  [../]
  [./sxx_second]
    type = RankTwoAux
    index_i = 0
    index_j = 0
    variable = sxx_second
    rank_two_tensor = stress
  [../]
  [./syy_second]
    type = RankTwoAux
    index_i = 1
    index_j = 1
    variable = syy_second
    rank_two_tensor = stress
  [../]
  [./szz_second]
    type = RankTwoAux
    index_i = 2
    index_j = 2
    variable = szz_second
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
  console = true
  exodus = true
[]
