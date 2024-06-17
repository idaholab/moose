# Affine Plasticity Test for Transient Stress Eigenvalues with Stationary Eigenvectors

# This test is taken from K. Jamojjala, R. Brannon, A. Sadeghirad, J. Guilkey,
#  "Verification tests in solid mechanics," Engineering with Computers, Vol 31.,
#  p. 193-213.

# The test involves applying particular strains and expecting particular stresses.

# The material properties are:
#  Yield in shear     165 MPa
#  Shear modulus       79 GPa
#  Poisson's ratio    1/3

# The strains are:
#  Time        e11        e22        e33
#  0             0          0          0
#  1        -0.003     -0.003      0.006
#  2    -0.0103923          0  0.0103923

# The expected stresses are:
#  sigma11:
#   -474*t                             0 < t <= 0.201
#   -95.26                             0.201 < t <= 1
#   (189.4+0.1704*sqrt(a)-0.003242*a)
#   ---------------------------------  1 < t <= 2
#            1+0.00001712*a
#   -189.4                             t > 2 (paper erroneously gives a positive value)
#
#  sigma22:
#   -474*t                             0 < t <= 0.201
#   -95.26                             0.201 < t <= 1
#   -(76.87+1.443*sqrt(a)-0.001316*a)
#   ---------------------------------  1 < t <= 2 (paper gives opposite sign)
#             1+0.00001712*a
#   76.87                              t > 2
#
#  sigma33:
#   948*t                              0 < t <= 0.201
#   190.5                              0.201 < t <= 1
#   -(112.5-1.272*sqrt(a)-0.001926*a)
#   ---------------------------------  1 < t <= 2 (paper has two sign errors here)
#            1+0.00001712*a
#   112.5                              t > 2
#
#  where a = exp(12.33*t).
#
# Note: If planning to run this case with strain type ComputeFiniteStrain, the
#   displacement function must be adjusted.  Instead of
#     strain = (l - l0)/l0 = (u+l0 - l0)/l0 = u/l0
#   with l0=1.0, we would have
#     strain = log(l/l0) = log((u+l0)/l0)
#   with l0=1.0.  So, for strain = -0.003,
#     -0.003 = log((u+l0)/l0) ->
#     u = exp(-0.003)*l0 - l0 = -0.0029955044966269995.
#


[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  block = '0'
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  # This test uses ElementalVariableValue postprocessors on specific
  # elements, so element numbering needs to stay unchanged
  allow_renumbering = false
[]

[Functions]
  [disp_x]
    type = PiecewiseLinear
    x = '0.  1.     2.'
    y = '0. -0.003 -0.0103923'
  []
  [disp_y]
    type = PiecewiseLinear
    x = '0.  1.    2.'
    y = '0. -0.003 0.'
  []
  [disp_z]
    type = PiecewiseLinear
    x = '0. 1.    2.'
    y = '0. 0.006 0.0103923'
  []
  [stress_xx]
    type = ParsedFunction
    # The paper gives 0.201 as the time at initial yield, but 0.20097635952803425 is the exact value.
    # The paper gives -95.26 MPa as the stress at yield, but -95.26279441628823 is the exact value.
    # The paper gives 12.33 as the factor in the exponential, but 12.332921390339125 is the exact value.
    # 189.409039923814000, 0.170423791206825, -0.003242011311945, 1.711645501845780E-05 - exact values
    symbol_names = 'timeAtYield stressAtYield expFac a b c d'
    symbol_values = '0.20097635952803425 -95.26279441628823 12.332921390339125 189.409039923814000 0.170423791206825 -0.003242011311945 1.711645501845780E-05'
    value = '1e6*
             if(t<=timeAtYield, -474*t,
             if(t<=1, stressAtYield,
             (a+b*sqrt(exp(expFac*t))+c*exp(expFac*t))/(1.0+d*exp(expFac*t))))' # tends to -a
  []
  [stress_yy]
    type = ParsedFunction
    # The paper gives 0.201 as the time at initial yield, but 0.20097635952803425 is the exact value.
    # the paper gives -95.26 MPa as the stress at yield, but -95.26279441628823 is the exact value.
    # The paper gives 12.33 as the factor in the exponential, but 12.332921390339125 is the exact value.
    # -76.867432297315000, -1.442488120272900, 0.001315697947301, 1.711645501845780E-05 - exact values
    symbol_names = 'timeAtYield stressAtYield expFac a b c d'
    symbol_values = '0.20097635952803425 -95.26279441628823 12.332921390339125 -76.867432297315000 -1.442488120272900 0.001315697947301 1.711645501845780E-05'
    value = '1e6*
             if(t<=timeAtYield, -474*t,
             if(t<=1, stressAtYield,
             (a+b*sqrt(exp(expFac*t))+c*exp(expFac*t))/(1.0+d*exp(expFac*t))))' # tends to -a
  []
  [stress_zz]
    type = ParsedFunction
    # The paper gives 0.201 as the time at initial yield, but 0.20097635952803425 is the exact value.
    # the paper gives 190.5 MPa as the stress at yield, but 190.52558883257645 is the exact value.
    # The paper gives 12.33 as the factor in the exponential, but 12.332921390339125 is the exact value.
    # -112.541607626499000, 1.272064329066080, 0.001926313364644, 1.711645501845780E-05 - exact values
    symbol_names = 'timeAtYield stressAtYield expFac a b c d'
    symbol_values = '0.20097635952803425 190.52558883257645 12.332921390339125 -112.541607626499000 1.272064329066080 0.001926313364644 1.711645501845780E-05'
    value = '1e6*
             if(t<=timeAtYield, 948*t,
             if(t<=1, stressAtYield,
             (a+b*sqrt(exp(expFac*t))+c*exp(expFac*t))/(1.0+d*exp(expFac*t))))' # tends to -a
  []
[]

[Variables]
  [disp_x]
    order = FIRST
    family = LAGRANGE
  []
  [disp_y]
    order = FIRST
    family = LAGRANGE
  []
  [disp_z]
    order = FIRST
    family = LAGRANGE
  []
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
  [./vonmises]
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

[Kernels]
  [SolidMechanics]
  [../]
[]

[AuxKernels]
  [./stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
    execute_on = 'timestep_end'
  [../]
  [./stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
    execute_on = 'timestep_end'
  [../]
  [./stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zz
    index_i = 2
    index_j = 2
    execute_on = 'timestep_end'
  [../]
  [./vonmises]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    variable = vonmises
    scalar_type = vonmisesStress
    execute_on = 'timestep_end'
  [../]

  [./plastic_strain_xx]
    type = RankTwoAux
    rank_two_tensor = plastic_strain
    variable = plastic_strain_xx
    index_i = 0
    index_j = 0
    execute_on = 'timestep_end'
  [../]
  [./plastic_strain_yy]
    type = RankTwoAux
    rank_two_tensor = plastic_strain
    variable = plastic_strain_yy
    index_i = 1
    index_j = 1
    execute_on = 'timestep_end'
  [../]
  [./plastic_strain_zz]
    type = RankTwoAux
    rank_two_tensor = plastic_strain
    variable = plastic_strain_zz
    index_i = 2
    index_j = 2
    execute_on = 'timestep_end'
  [../]
[]

[BCs]
  [fixed_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  []
  [fixed_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []
  [fixed_z]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0.0
  []

  [disp_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = right
    function = disp_x
  []
  [disp_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = top
    function = disp_y
  []
  [disp_z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = front
    function = disp_z
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 210666666666.666667
    poissons_ratio = 0.3333333333333333
  [../]

  [./strain]
    type = ComputeIncrementalStrain
  [../]

  [creep]
    type = PowerLawCreepStressUpdate
    coefficient = 0
    n_exponent = 1
    m_exponent = 1
    activation_energy = 0
    temperature = 1
  []
  [isotropic_plasticity]
    type = IsotropicPlasticityStressUpdate
    yield_stress = 285788383.2488647 # = sqrt(3)*165e6 = sqrt(3) * yield in shear
    hardening_constant = 0.0
  []
  [radial_return_stress]
    type = ComputeCreepPlasticityStress
    tangent_operator = elastic
    creep_model = creep
    plasticity_model = isotropic_plasticity
  []
[]

[Executioner]

  type = Transient

  solve_type = 'PJFNK'
  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  nl_abs_tol = 1e-10

  l_max_its = 20

  start_time = 0.0
  dt = 0.01 # use 0.0001 for a nearly exact match
  end_time = 2.0
[]

[Postprocessors]
  [analytic_xx]
    type = FunctionValuePostprocessor
    function = stress_xx
  []
  [analytic_yy]
    type = FunctionValuePostprocessor
    function = stress_yy
  []
  [analytic_zz]
    type = FunctionValuePostprocessor
    function = stress_zz
  []

  [stress_xx]
    type = ElementalVariableValue
    variable = stress_xx
    elementid = 0
  []
  [stress_yy]
    type = ElementalVariableValue
    variable = stress_yy
    elementid = 0
  []
  [stress_zz]
    type = ElementalVariableValue
    variable = stress_zz
    elementid = 0
  []

  [stress_xx_l2_error]
    type = ElementL2Error
    variable = stress_xx
    function = stress_xx
  []
  [stress_yy_l2_error]
    type = ElementL2Error
    variable = stress_yy
    function = stress_yy
  []
  [stress_zz_l2_error]
    type = ElementL2Error
    variable = stress_zz
    function = stress_zz
  []
[]

[Outputs]
  exodus = true
[]
