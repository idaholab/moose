#
# KKS coupled with elasticity. Physical parameters for matrix and precipitate phases
# are gamma and gamma-prime phases, respectively, in the Ni-Al system.
# Parameterization is as described in L.K. Aagesen et al., Computational Materials
# Science, 140, 10-21 (2017), with isotropic elastic properties in both phases
# and without eigenstrain.
#

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 200
  xmax = 200
[]

[Problem]
  coord_type = RSPHERICAL
[]

[GlobalParams]
  displacements = 'disp_x'
[]

[Variables]
  # order parameter
  [./eta]
    order = FIRST
    family = LAGRANGE
  [../]

  # solute concentration
  [./c]
    order = FIRST
    family = LAGRANGE
  [../]

  # chemical potential
  [./w]
    order = FIRST
    family = LAGRANGE
  [../]

  # solute phase concentration (matrix)
  [./cm]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.13
  [../]
  # solute phase concentration (precipitate)
  [./cp]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.235
  [../]
[]

[AuxVariables]
  [./energy_density]
    family = MONOMIAL
  [../]
  [./extra_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./extra_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./extra_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[ICs]
  [./eta_ic]
    variable = eta
    type = FunctionIC
    function = ic_func_eta
  [../]
  [./c_ic]
    variable = c
    type = FunctionIC
    function = ic_func_c
  [../]
[]

[Functions]
  [./ic_func_eta]
    type = ParsedFunction
    expression = 'r:=sqrt(x^2+y^2+z^2);0.5*(1.0-tanh((r-r0)/delta_eta/sqrt(2.0)))'
    symbol_names = 'delta_eta r0'
    symbol_values = '6.431     100'
  [../]
  [./ic_func_c]
    type = ParsedFunction
    expression = 'r:=sqrt(x^2+y^2+z^2);eta_an:=0.5*(1.0-tanh((r-r0)/delta/sqrt(2.0)));0.235*eta_an^3*(6*eta_an^2-15*eta_an+10)+0.13*(1-eta_an^3*(6*eta_an^2-15*eta_an+10))'
    symbol_names = 'delta r0'
    symbol_values = '6.431 100'
  [../]
[]

[Physics/SolidMechanics/QuasiStatic]
  [./all]
    add_variables = true
    generate_output = 'hydrostatic_stress stress_xx stress_yy stress_zz'
  [../]
[]


[Kernels]

  # enforce c = (1-h(eta))*cm + h(eta)*cp
  [./PhaseConc]
    type = KKSPhaseConcentration
    ca       = cm
    variable = cp
    c        = c
    eta      = eta
  [../]

  # enforce pointwise equality of chemical potentials
  [./ChemPotVacancies]
    type = KKSPhaseChemicalPotential
    variable = cm
    cb       = cp
    fa_name  = f_total_matrix
    fb_name  = f_total_ppt
  [../]

  #
  # Cahn-Hilliard Equation
  #
  [./CHBulk]
    type = KKSSplitCHCRes
    variable = c
    ca       = cm
    fa_name  = f_total_matrix
    w        = w
  [../]

  [./dcdt]
    type = CoupledTimeDerivative
    variable = w
    v = c
  [../]
  [./ckernel]
    type = SplitCHWRes
    mob_name = M
    variable = w
  [../]

  #
  # Allen-Cahn Equation
  #
  [./ACBulkF]
    type = KKSACBulkF
    variable = eta
    fa_name  = f_total_matrix
    fb_name  = f_total_ppt
    w        = 0.0033
    args = 'cp cm'
  [../]
  [./ACBulkC]
    type = KKSACBulkC
    variable = eta
    ca       = cm
    cb       = cp
    fa_name  = f_total_matrix
  [../]
  [./ACInterface]
    type = ACInterface
    variable = eta
    kappa_name = kappa
  [../]
  [./detadt]
    type = TimeDerivative
    variable = eta
  [../]
[]

[AuxKernels]
  [./extra_xx]
    type = RankTwoAux
    rank_two_tensor = extra_stress
    index_i = 0
    index_j = 0
    variable = extra_xx
  [../]
  [./extra_yy]
    type = RankTwoAux
    rank_two_tensor = extra_stress
    index_i = 1
    index_j = 1
    variable = extra_yy
  [../]
  [./extra_zz]
    type = RankTwoAux
    rank_two_tensor = extra_stress
    index_i = 2
    index_j = 2
    variable = extra_zz
  [../]
  [./strain_xx]
    type = RankTwoAux
    rank_two_tensor = mechanical_strain
    index_i = 0
    index_j = 0
    variable = strain_xx
  [../]
  [./strain_yy]
    type = RankTwoAux
    rank_two_tensor = mechanical_strain
    index_i = 1
    index_j = 1
    variable = strain_yy
  [../]
  [./strain_zz]
    type = RankTwoAux
    rank_two_tensor = mechanical_strain
    index_i = 2
    index_j = 2
    variable = strain_zz
  [../]
[]

[Materials]
  # Chemical free energy of the matrix
  [./fm]
    type = DerivativeParsedMaterial
    property_name = fm
    coupled_variables = 'cm'
    expression = '6.55*(cm-0.13)^2'
  [../]
# Elastic energy of the matrix
  [./elastic_free_energy_m]
    type = ElasticEnergyMaterial
    base_name = matrix
    f_name = fe_m
    args = ' '
  [../]
# Total free energy of the matrix
  [./Total_energy_matrix]
    type = DerivativeSumMaterial
    property_name = f_total_matrix
    sum_materials = 'fm fe_m'
    coupled_variables = 'cm'
  [../]

  # Free energy of the precipitate phase
  [./fp]
    type = DerivativeParsedMaterial
    property_name = fp
    coupled_variables = 'cp'
    expression = '6.55*(cp-0.235)^2'
  [../]

# Elastic energy of the precipitate
  [./elastic_free_energy_p]
    type = ElasticEnergyMaterial
    base_name = ppt
    f_name = fe_p
    args = ' '
  [../]

# Total free energy of the precipitate
  [./Total_energy_ppt]
    type = DerivativeSumMaterial
    property_name = f_total_ppt
    sum_materials = 'fp fe_p'
    coupled_variables = 'cp'
  [../]

# Total elastic energy
  [./Total_elastic_energy]
    type = DerivativeTwoPhaseMaterial
    eta = eta
    f_name = f_el_mat
    fa_name = fe_m
    fb_name = fe_p
    outputs = exodus
    W = 0
  [../]

  # h(eta)
  [./h_eta]
    type = SwitchingFunctionMaterial
    h_order = HIGH
    eta = eta
  [../]

  # g(eta)
  [./g_eta]
    type = BarrierFunctionMaterial
    g_order = SIMPLE
    eta = eta
    outputs = exodus
  [../]

  # constant properties
  [./constants]
    type = GenericConstantMaterial
    prop_names  = 'M   L   kappa'
    prop_values = '0.7 0.7 0.1365'
  [../]

  #Mechanical properties
  [./Stiffness_matrix]
    type = ComputeElasticityTensor
    C_ijkl = '74.25 14.525'
    base_name = matrix
    fill_method = symmetric_isotropic
  [../]
  [./Stiffness_ppt]
    type = ComputeElasticityTensor
    C_ijkl = '74.25 14.525'
    base_name = ppt
    fill_method = symmetric_isotropic
  [../]
  [./strain_matrix]
    type = ComputeRSphericalSmallStrain
    base_name = matrix
  [../]
  [./strain_ppt]
    type = ComputeRSphericalSmallStrain
    base_name = ppt
  [../]
  [./stress_matrix]
    type = ComputeLinearElasticStress
    base_name = matrix
  [../]
  [./stress_ppt]
    type = ComputeLinearElasticStress
    base_name = ppt
  [../]
  [./global_stress]
    type = TwoPhaseStressMaterial
    base_A = matrix
    base_B = ppt
  [../]
  [./interface_stress]
    type = ComputeSurfaceTensionKKS
    v = eta
    kappa_name = kappa
    w = 0.0033
  [../]
[]

[BCs]
  [./left_r]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  [../]
[]

#
# Precondition using handcoded off-diagonal terms
#
[Preconditioning]
  [./full]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -sub_pc_type   -sub_pc_factor_shift_type'
  petsc_options_value = 'asm       lu            nonzero'

  l_max_its = 30
  nl_max_its = 10
  l_tol = 1.0e-4
  nl_rel_tol = 1.0e-9
  nl_abs_tol = 1.0e-10

  num_steps = 2
  dt = 0.5
[]

[Outputs]
  exodus = true
  [./csv]
    type = CSV
    execute_on = 'final'
  [../]
[]
