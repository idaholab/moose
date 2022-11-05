# KKS phase-field model coupled with elasticity using the Voigt-Taylor scheme as
# described in L.K. Aagesen et al., Computational Materials Science, 140, 10-21 (2017)
# Original run #170329e

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 640
  ny = 1
  nz = 1
  xmin = -10
  xmax = 10
  ymin = 0
  ymax = 0.03125
  zmin = 0
  zmax = 0.03125
  elem_type = HEX8
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
  [../]
  # solute phase concentration (precipitate)
  [./cp]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_z]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[ICs]
  [./eta_ic]
    variable = eta
    type = FunctionIC
    function = ic_func_eta
    block = 0
  [../]
  [./c_ic]
    variable = c
    type = FunctionIC
    function = ic_func_c
    block = 0
  [../]
  [./w_ic]
    variable = w
    type = ConstantIC
    value = 0.00991
    block = 0
  [../]
  [./cm_ic]
    variable = cm
    type = ConstantIC
    value = 0.131
    block = 0
  [../]
  [./cp_ic]
    variable = cp
    type = ConstantIC
    value = 0.236
    block = 0
  [../]
[]

[Functions]
  [./ic_func_eta]
    type = ParsedFunction
    expression = '0.5*(1.0+tanh((x)/delta_eta/sqrt(2.0)))'
    symbol_names = 'delta_eta'
    symbol_values = '0.8034'
  [../]
  [./ic_func_c]
    type = ParsedFunction
    expression = '0.2388*(0.5*(1.0+tanh(x/delta/sqrt(2.0))))^3*(6*(0.5*(1.0+tanh(x/delta/sqrt(2.0))))^2-15*(0.5*(1.0+tanh(x/delta/sqrt(2.0))))+10)+0.1338*(1-(0.5*(1.0+tanh(x/delta/sqrt(2.0))))^3*(6*(0.5*(1.0+tanh(x/delta/sqrt(2.0))))^2-15*(0.5*(1.0+tanh(x/delta/sqrt(2.0))))+10))'
    symbol_names = 'delta'
    symbol_values = '0.8034'
  [../]
  [./psi_eq_int]
    type = ParsedFunction
    expression = 'volume*psi_alpha'
    symbol_names = 'volume psi_alpha'
    symbol_values = 'volume psi_alpha'
  [../]
  [./gamma]
    type = ParsedFunction
    expression = '(psi_int - psi_eq_int) / dy / dz'
    symbol_names = 'psi_int psi_eq_int dy       dz'
    symbol_values = 'psi_int psi_eq_int 0.03125  0.03125'
  [../]
[]

[AuxVariables]
  [./sigma11]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./sigma22]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./sigma33]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./e11]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./e12]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./e22]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./e33]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./e_el11]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./e_el12]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./e_el22]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./f_el]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./eigen_strain00]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./Fglobal]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./psi]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./matl_sigma11]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 0
    index_j = 0
    variable = sigma11
  [../]
  [./matl_sigma22]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 1
    index_j = 1
    variable = sigma22
  [../]
  [./matl_sigma33]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 2
    index_j = 2
    variable = sigma33
  [../]
  [./matl_e11]
    type = RankTwoAux
    rank_two_tensor = total_strain
    index_i = 0
    index_j = 0
    variable = e11
  [../]
  [./matl_e12]
    type = RankTwoAux
    rank_two_tensor = total_strain
    index_i = 0
    index_j = 1
    variable = e12
  [../]
  [./matl_e22]
    type = RankTwoAux
    rank_two_tensor = total_strain
    index_i = 1
    index_j = 1
    variable = e22
  [../]
  [./matl_e33]
    type = RankTwoAux
    rank_two_tensor = total_strain
    index_i = 2
    index_j = 2
    variable = e33
  [../]
  [./f_el]
    type = MaterialRealAux
    variable = f_el
    property = f_el_mat
    execute_on = timestep_end
  [../]
  [./GlobalFreeEnergy]
    variable = Fglobal
    type = KKSGlobalFreeEnergy
    fa_name = fm
    fb_name = fp
    w = 0.0264
    kappa_names = kappa
    interfacial_vars = eta
  [../]
  [./psi_potential]
    variable = psi
    type = ParsedAux
    coupled_variables = 'Fglobal w c f_el sigma11 e11'
    expression = 'Fglobal - w*c + f_el - sigma11*e11'
  [../]
[]


[BCs]
  [./left_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  [../]
  [./right_x]
    type = DirichletBC
    variable = disp_x
    boundary = right
    value = 0
  [../]
  [./front_y]
    type = DirichletBC
    variable = disp_y
    boundary = front
    value = 0
  [../]
  [./back_y]
    type = DirichletBC
    variable = disp_y
    boundary = back
    value = 0
  [../]
  [./top_z]
    type = DirichletBC
    variable = disp_z
    boundary = top
    value = 0
  [../]
  [./bottom_z]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value = 0
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
    outputs = exodus
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
    outputs = exodus
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
  [../]

  # constant properties
  [./constants]
    type = GenericConstantMaterial
    prop_names  = 'M   L   kappa     misfit'
    prop_values = '0.7 0.7 0.01704   0.00377'
  [../]

  #Mechanical properties
  [./Stiffness_matrix]
    type = ComputeElasticityTensor
    C_ijkl = '103.3 74.25 74.25 103.3 74.25 103.3 46.75 46.75 46.75'
    base_name = matrix
    fill_method = symmetric9
  [../]
  [./Stiffness_ppt]
    type = ComputeElasticityTensor
    C_ijkl = '100.7 71.45 71.45 100.7 71.45 100.7 50.10 50.10 50.10'
    base_name = ppt
    fill_method = symmetric9
  [../]
  [./stress_matrix]
    type = ComputeLinearElasticStress
    base_name = matrix
  [../]
  [./stress_ppt]
    type = ComputeLinearElasticStress
    base_name = ppt
  [../]
  [./strain_matrix]
    type = ComputeSmallStrain
    displacements = 'disp_x disp_y disp_z'
    base_name = matrix
  [../]
  [./strain_ppt]
    type = ComputeSmallStrain
    displacements = 'disp_x disp_y disp_z'
    base_name = ppt
    eigenstrain_names = 'eigenstrain_ppt'
  [../]
  [./eigen_strain]
    type = ComputeEigenstrain
    base_name = ppt
    eigen_base = '1 1 1 0 0 0'
    prefactor = misfit
    eigenstrain_name = 'eigenstrain_ppt'
  [../]
  [./global_stress]
    type = TwoPhaseStressMaterial
    base_A = matrix
    base_B = ppt
  [../]
  [./global_strain]
    type = ComputeSmallStrain
    displacements = 'disp_x disp_y disp_z'
  [../]
[]

[Kernels]
  [./TensorMechanics]
    displacements = 'disp_x disp_y disp_z'
  [../]

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
    w        = 0.0264
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

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -sub_pc_type   -sub_pc_factor_shift_type'
  petsc_options_value = 'asm       ilu            nonzero'

  l_max_its = 30
  nl_max_its = 10
  l_tol = 1.0e-4
  nl_rel_tol = 1.0e-8
  nl_abs_tol = 1.0e-11
  num_steps = 200

  [./TimeStepper]
    type = SolutionTimeAdaptiveDT
    dt = 0.5
  [../]
[]

[VectorPostprocessors]
  #[./eta]
  #  type =  LineValueSampler
  #  start_point = '-10 0 0'
  #  end_point = '10 0 0'
  #  variable = eta
  #  num_points = 321
  #  sort_by =  id
  #[../]
  #[./eta_position]
  #  type = FindValueOnLineSample
  #  vectorpostprocessor = eta
  #  variable_name = eta
  #  search_value = 0.5
  #[../]
#  [./f_el]
#    type =  LineMaterialRealSampler
#    start = '-20 0 0'
#    end   = '20 0 0'
#    sort_by = id
#    property = f_el
#  [../]
#  [./f_el_a]
#    type =  LineMaterialRealSampler
#    start = '-20 0 0'
#    end   = '20 0 0'
#    sort_by = id
#    property = fe_m
#  [../]
#  [./f_el_b]
#    type =  LineMaterialRealSampler
#    start = '-20 0 0'
#    end   = '20 0 0'
#    sort_by = id
#    property = fe_p
#  [../]
#  [./h_out]
#    type =  LineMaterialRealSampler
#    start = '-20 0 0'
#    end   = '20 0 0'
#    sort_by = id
#    property = h
#  [../]
#  [./fm_out]
#    type =  LineMaterialRealSampler
#    start = '-20 0 0'
#    end   = '20 0 0'
#    sort_by = id
#    property = fm
#  [../]
[]

[Postprocessors]
  [./f_el_int]
    type = ElementIntegralMaterialProperty
    mat_prop = f_el_mat
  [../]
  [./c_alpha]
    type =  SideAverageValue
    boundary = left
    variable = c
  [../]
  [./c_beta]
    type =  SideAverageValue
    boundary = right
    variable = c
  [../]
  [./e11_alpha]
    type =  SideAverageValue
    boundary = left
    variable = e11
  [../]
  [./e11_beta]
    type =  SideAverageValue
    boundary = right
    variable = e11
  [../]
  [./s11_alpha]
    type =  SideAverageValue
    boundary = left
    variable = sigma11
  [../]
  [./s22_alpha]
    type =  SideAverageValue
    boundary = left
    variable = sigma22
  [../]
  [./s33_alpha]
    type =  SideAverageValue
    boundary = left
    variable = sigma33
  [../]
  [./s11_beta]
    type =  SideAverageValue
    boundary = right
    variable = sigma11
  [../]
  [./s22_beta]
    type =  SideAverageValue
    boundary = right
    variable = sigma22
  [../]
  [./s33_beta]
    type =  SideAverageValue
    boundary = right
    variable = sigma33
  [../]
  [./f_el_alpha]
    type =  SideAverageValue
    boundary = left
    variable = f_el
  [../]
  [./f_el_beta]
    type =  SideAverageValue
    boundary = right
    variable = f_el
  [../]
  [./f_c_alpha]
    type =  SideAverageValue
    boundary = left
    variable = Fglobal
  [../]
  [./f_c_beta]
    type =  SideAverageValue
    boundary = right
    variable = Fglobal
  [../]
  [./chem_pot_alpha]
    type =  SideAverageValue
    boundary = left
    variable = w
  [../]
  [./chem_pot_beta]
    type =  SideAverageValue
    boundary = right
    variable = w
  [../]
  [./psi_alpha]
    type =  SideAverageValue
    boundary = left
    variable = psi
  [../]
  [./psi_beta]
    type =  SideAverageValue
    boundary = right
    variable = psi
  [../]
  [./total_energy]
    type = ElementIntegralVariablePostprocessor
    variable = Fglobal
  [../]
  # Get simulation cell size from postprocessor
  [./volume]
    type = ElementIntegralMaterialProperty
    mat_prop = 1
  [../]
  [./psi_eq_int]
    type = FunctionValuePostprocessor
    function = psi_eq_int
  [../]
  [./psi_int]
    type = ElementIntegralVariablePostprocessor
    variable = psi
  [../]
  [./gamma]
    type = FunctionValuePostprocessor
    function = gamma
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


[Outputs]
  [./exodus]
    type = Exodus
    interval = 20
  [../]
  [./csv]
    type = CSV
    execute_on = 'final'
  [../]
#[./console]
#    type = Console
#    output_file = true
#  [../]
[]
