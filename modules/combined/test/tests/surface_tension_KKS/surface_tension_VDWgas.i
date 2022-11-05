# Test for ComputeExtraStressVDWGas
# Gas bubble with r = 15 nm in a solid matrix
# The gas pressure is counterbalanced by the surface tension of the solid-gas interface,
# which is included with ComputeSurfaceTensionKKS

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 300
  xmin = 0
  xmax = 30
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

  # gas concentration
  [./cg]
    order = FIRST
    family = LAGRANGE
  [../]

  # vacancy concentration
  [./cv]
    order = FIRST
    family = LAGRANGE
  [../]


  # gas chemical potential
  [./wg]
    order = FIRST
    family = LAGRANGE
  [../]

  # vacancy chemical potential
  [./wv]
    order = FIRST
    family = LAGRANGE
  [../]


  # Matrix phase gas concentration
  [./cgm]
    order = FIRST
    family = LAGRANGE
    initial_condition = 1.01e-31
  [../]

  # Matrix phase vacancy concentration
  [./cvm]
    order = FIRST
    family = LAGRANGE
    initial_condition = 2.25e-11
  [../]

  # Bubble phase gas concentration
  [./cgb]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.2714
  [../]

  # Bubble phase vacancy concentration
  [./cvb]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.7286
  [../]
[]

[ICs]
  [./eta_ic]
    variable = eta
    type = FunctionIC
    function = ic_func_eta
  [../]
  [./cv_ic]
    variable = cv
    type = FunctionIC
    function = ic_func_cv
  [../]
  [./cg_ic]
    variable = cg
    type = FunctionIC
    function = ic_func_cg
  [../]
[]

[Functions]
  [./ic_func_eta]
    type = ParsedFunction
    expression = 'r:=sqrt(x^2+y^2+z^2);0.5*(1.0-tanh((r-r0)/delta_eta/sqrt(2.0)))'
    symbol_names = 'delta_eta r0'
    symbol_values = '0.321     15'
  [../]
  [./ic_func_cv]
    type = ParsedFunction
    expression = 'r:=sqrt(x^2+y^2+z^2);eta_an:=0.5*(1.0-tanh((r-r0)/delta/sqrt(2.0)));cvbubinit*eta_an^3*(6*eta_an^2-15*eta_an+10)+cvmatrixinit*(1-eta_an^3*(6*eta_an^2-15*eta_an+10))'
    symbol_names = 'delta r0  cvbubinit cvmatrixinit'
    symbol_values = '0.321 15  0.7286    2.25e-11'
  [../]
  [./ic_func_cg]
    type = ParsedFunction
    expression = 'r:=sqrt(x^2+y^2+z^2);eta_an:=0.5*(1.0-tanh((r-r0)/delta/sqrt(2.0)));cgbubinit*eta_an^3*(6*eta_an^2-15*eta_an+10)+cgmatrixinit*(1-eta_an^3*(6*eta_an^2-15*eta_an+10))'
    symbol_names = 'delta r0  cgbubinit cgmatrixinit'
    symbol_values = '0.321 15  0.2714    1.01e-31'
  [../]
[]



[Modules/TensorMechanics/Master]
  [./all]
    add_variables = true
    generate_output = 'hydrostatic_stress stress_xx stress_yy stress_zz'
  [../]
[]

[Kernels]
  # enforce cg = (1-h(eta))*cgm + h(eta)*cgb
  [./PhaseConc_g]
    type = KKSPhaseConcentration
    ca       = cgm
    variable = cgb
    c        = cg
    eta      = eta
  [../]

  # enforce cv = (1-h(eta))*cvm + h(eta)*cvb
  [./PhaseConc_v]
    type = KKSPhaseConcentration
    ca       = cvm
    variable = cvb
    c        = cv
    eta      = eta
  [../]

  # enforce pointwise equality of chemical potentials
  [./ChemPotVacancies]
    type = KKSPhaseChemicalPotential
    variable = cvm
    cb       = cvb
    fa_name  = f_total_matrix
    fb_name  = f_total_bub
    args_a = 'cgm'
    args_b = 'cgb'
  [../]
  [./ChemPotGas]
    type = KKSPhaseChemicalPotential
    variable = cgm
    cb       = cgb
    fa_name  = f_total_matrix
    fb_name  = f_total_bub
    args_a = 'cvm'
    args_b = 'cvb'
  [../]

  #
  # Cahn-Hilliard Equations
  #
  [./CHBulk_g]
    type = KKSSplitCHCRes
    variable = cg
    ca       = cgm
    fa_name  = f_total_matrix
    w        = wg
    args_a   = 'cvm'
  [../]
  [./CHBulk_v]
    type = KKSSplitCHCRes
    variable = cv
    ca       = cvm
    fa_name  = f_total_matrix
    w        = wv
    args_a   = 'cgm'
  [../]

  [./dcgdt]
    type = CoupledTimeDerivative
    variable = wg
    v = cg
  [../]
  [./dcvdt]
    type = CoupledTimeDerivative
    variable = wv
    v = cv
  [../]

  [./wgkernel]
    type = SplitCHWRes
    mob_name = M
    variable = wg
  [../]
  [./wvkernel]
    type = SplitCHWRes
    mob_name = M
    variable = wv
  [../]

  #
  # Allen-Cahn Equation
  #
  [./ACBulkF]
    type = KKSACBulkF
    variable = eta
    fa_name  = f_total_matrix
    fb_name  = f_total_bub
    w        = 0.356
    args = 'cvm cvb cgm cgb'
  [../]
  [./ACBulkCv]
    type = KKSACBulkC
    variable = eta
    ca       = cvm
    cb       = cvb
    fa_name  = f_total_matrix
    args     = 'cgm'
  [../]
  [./ACBulkCg]
    type = KKSACBulkC
    variable = eta
    ca       = cgm
    cb       = cgb
    fa_name  = f_total_matrix
    args     = 'cvm'
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

[Materials]
  # Chemical free energy of the matrix
  [./fm]
    type = DerivativeParsedMaterial
    property_name = fm
    coupled_variables = 'cvm cgm'
    material_property_names = 'kvmatrix kgmatrix cvmatrixeq cgmatrixeq'
    expression = '0.5*kvmatrix*(cvm-cvmatrixeq)^2 + 0.5*kgmatrix*(cgm-cgmatrixeq)^2'
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
    coupled_variables = 'cvm cgm'
  [../]

  # Free energy of the bubble phase
  [./fb]
    type = DerivativeParsedMaterial
    property_name = fb
    coupled_variables = 'cvb cgb'
    material_property_names = 'kToverV nQ Va b f0 kpen kgbub kvbub cvbubeq cgbubeq'
    expression = '0.5*kgbub*(cvb-cvbubeq)^2 + 0.5*kvbub*(cgb-cgbubeq)^2'
  [../]

# Elastic energy of the bubble
  [./elastic_free_energy_p]
    type = ElasticEnergyMaterial
    base_name = bub
    f_name = fe_b
    args = ' '
  [../]

# Total free energy of the bubble
  [./Total_energy_bub]
    type = DerivativeSumMaterial
    property_name = f_total_bub
    sum_materials = 'fb fe_b'
    # sum_materials = 'fb'
    coupled_variables = 'cvb cgb'
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
    prop_names  = 'M   L   kappa  Va      kvmatrix  kgmatrix  kgbub kvbub f0      kpen  cvbubeq cgbubeq b      T'
    prop_values = '0.7 0.7 0.0368 0.03629 223.16    223.16    2.23  2.23  0.0224  1.0   0.6076  0.3924  0.085  800'
  [../]
  [./cvmatrixeq]
    type = ParsedMaterial
    property_name = cvmatrixeq
    material_property_names = 'T'
    constant_names        = 'kB           Efv'
    constant_expressions  = '8.6173324e-5 1.69'
    expression = 'exp(-Efv/(kB*T))'
  [../]
  [./cgmatrixeq]
    type = ParsedMaterial
    property_name = cgmatrixeq
    material_property_names = 'T'
    constant_names        = 'kB           Efg'
    constant_expressions  = '8.6173324e-5 4.92'
    expression = 'exp(-Efg/(kB*T))'
  [../]
  [./kToverV]
    type = ParsedMaterial
    property_name = kToverV
    material_property_names = 'T Va'
    constant_names        = 'k          C44dim' #k in J/K and dimensional C44 in J/m^3
    constant_expressions  = '1.38e-23   63e9'
    expression = 'k*T*1e27/Va/C44dim'
  [../]
  [./nQ]
    type = ParsedMaterial
    property_name = nQ
    material_property_names = 'T'
    constant_names        = 'k          Pi      M         hbar' #k in J/K, M is Xe atomic mass in kg, hbar in J s
    constant_expressions  = '1.38e-23   3.14159 2.18e-25  1.05459e-34'
    expression = '(M*k*T/2/Pi/hbar^2)^1.5 * 1e-27' #1e-27 converts from #/m^3 to #/nm^3
  [../]

  #Mechanical properties
  [./Stiffness_matrix]
    type = ComputeElasticityTensor
    C_ijkl = '0.778 0.7935'
    fill_method = symmetric_isotropic
    base_name = matrix
  [../]
  [./Stiffness_bub]
    type = ComputeElasticityTensor
    C_ijkl = '0.0778 0.07935'
    fill_method = symmetric_isotropic
    base_name = bub
  [../]
  [./strain_matrix]
    type = ComputeRSphericalSmallStrain
    base_name = matrix
  [../]
  [./strain_bub]
    type = ComputeRSphericalSmallStrain
    base_name = bub
  [../]
  [./stress_matrix]
    type = ComputeLinearElasticStress
    base_name = matrix
  [../]
  [./stress_bub]
    type = ComputeLinearElasticStress
    base_name = bub
  [../]
  [./global_stress]
    type = TwoPhaseStressMaterial
    base_A = matrix
    base_B = bub
  [../]
  [./surface_tension]
    type = ComputeSurfaceTensionKKS
    v = eta
    kappa_name = kappa
    w = 0.356
  [../]
  [./gas_pressure]
    type = ComputeExtraStressVDWGas
    T = T
    b = b
    cg = cgb
    Va = Va
    nondim_factor = 63e9
    base_name = bub
    outputs = exodus
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
  nl_max_its = 15
  l_tol = 1.0e-4
  nl_rel_tol = 1.0e-10
  nl_abs_tol = 1e-11
  num_steps = 2
  dt = 0.5
[]

[Outputs]
  exodus = true
[]
