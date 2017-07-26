#
# This test exercises the smoothing of eigenstrains.
#
# The change in temperature is -150*r + 200.
#
# Young's modulus is 1e8.  Poisson's ratio is zero.  The coefficient of
# thermal expansion is 1e-6.
#
# All nodes are restrained in all directions.
#
# The mesh is a rectangular domain 1 unit high starting at r=0 and ending
# at r=2.  The first element's width is 1/2.  The elements are linear.
#
# The eigenstrain smoothing forces all eigenstrains to be constant in an
# element for linear elements using volume averaging.  For the given change in
# temperature, the average change in temperature is
# 2*pi*int((-150*r+200)*r)/(2*pi*int(r)) = 2*(-50*r*r*r+100*r*r)[a,b]/(r*r)[a,b]
# which leads to 150 over (0,1/2) and -10 over (1/2,2).
#
#                      Element #1   Element #2
# --------------------------------------------
#  Ave thermal strain   150e-6      -10e-6
#  stress_xx           -15000        1000
#  stress_yy           -15000        1000
#  stress_zz           -15000        1000
#

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Problem]
  coord_type = RZ
[]

[Mesh]
  file = eigenRZ.e
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[AuxVariables]
  [./sxx_constant]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./sxx_first]
    order = FIRST
    family = MONOMIAL
  [../]
  [./sxx_second]
    order = SECOND
    family = MONOMIAL
  [../]
  [./syy_constant]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./syy_first]
    order = FIRST
    family = MONOMIAL
  [../]
  [./syy_second]
    order = SECOND
    family = MONOMIAL
  [../]
  [./szz_constant]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./szz_first]
    order = FIRST
    family = MONOMIAL
  [../]
  [./szz_second]
    order = SECOND
    family = MONOMIAL
  [../]
  [./hydro_constant]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./hydro_first]
    order = FIRST
    family = MONOMIAL
  [../]
  [./hydro_second]
    order = SECOND
    family = MONOMIAL
  [../]
  [./temp]
    order = FIRST
    family = LAGRANGE
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
  [./sxx_first]
    type = RankTwoAux
    index_i = 0
    index_j = 0
    variable = sxx_first
    rank_two_tensor = stress
  [../]
  [./sxx_second]
    type = RankTwoAux
    index_i = 0
    index_j = 0
    variable = sxx_second
    rank_two_tensor = stress
  [../]
  [./syy_constant]
    type = RankTwoAux
    index_i = 1
    index_j = 1
    variable = syy_constant
    rank_two_tensor = stress
  [../]
  [./syy_first]
    type = RankTwoAux
    index_i = 1
    index_j = 1
    variable = syy_first
    rank_two_tensor = stress
  [../]
  [./syy_second]
    type = RankTwoAux
    index_i = 1
    index_j = 1
    variable = syy_second
    rank_two_tensor = stress
  [../]
  [./szz_constant]
    type = RankTwoAux
    index_i = 2
    index_j = 2
    variable = szz_constant
    rank_two_tensor = stress
  [../]
  [./szz_first]
    type = RankTwoAux
    index_i = 2
    index_j = 2
    variable = szz_first
    rank_two_tensor = stress
  [../]
  [./szz_second]
    type = RankTwoAux
    index_i = 2
    index_j = 2
    variable = szz_second
    rank_two_tensor = stress
  [../]
  [./hydro_constant]
    type = RankTwoScalarAux
    variable = hydro_constant
    rank_two_tensor = stress
    scalar_type = hydrostatic
  [../]
  [./hydro_first]
    type = RankTwoScalarAux
    variable = hydro_first
    rank_two_tensor = stress
    scalar_type = hydrostatic
  [../]
  [./hydro_second]
    type = RankTwoScalarAux
    variable = hydro_second
    rank_two_tensor = stress
    scalar_type = hydrostatic
  [../]

  [./temp]
    type = FunctionAux
    variable = temp
    function = linear_temp
  [../]
[]

[Functions]
  [./linear_temp]
    type = ParsedFunction
    value = '-150*x+500'
  [../]
[]

[BCs]
  [./fixed_x]
    type = PresetBC
    variable = disp_x
    value = 0
    boundary = 'bottom top'
  [../]
  [./fixed_y]
    type = PresetBC
    variable = disp_y
    value = 0
    boundary = 'bottom top'
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
    eigenstrain_names = 'eigenstrain'
  [../]
  [./stress]
    type = ComputeFiniteStrainElasticStress
  [../]
  [./thermal_expansion_strain]
    type = ComputeThermalExpansionEigenstrain
    stress_free_temperature = 300
    thermal_expansion_coeff = 1e-6
    temperature = temp
    incremental_form = true
    eigenstrain_name = eigenstrain
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
  csv = true
[]

[VectorPostprocessors]
  [./aux]
    type = LineValueSampler
    num_points = 41
    start_point = '0.0 0.5 0.0'
    end_point = '2.0 0.5 0.0'
    sort_by = x
    variable = 'hydro_constant hydro_first hydro_second sxx_constant sxx_first sxx_second syy_constant syy_first syy_second szz_constant szz_first szz_second temp'
  [../]
[]
