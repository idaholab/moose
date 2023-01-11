[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 800
  xmin = 0
  xmax = 80
[]

[GlobalParams]
  op_num = 2
  var_name_base = gr
  int_width = 4
[]

[Variables]
  [wvy]
  []
  [wvo]
  []
  [phi]
  []
  [PolycrystalVariables]
  []
  [V]
  []
[]

[AuxVariables]
  [bnds]
  []
  [negative_V]
  []
  [E_x]
    order = CONSTANT
    family = MONOMIAL
  []
  [E_y]
    order = CONSTANT
    family = MONOMIAL
  []
  [ns_cat_aux]
    order = CONSTANT
    family = MONOMIAL
  []
  [ns_an_aux]
    order = CONSTANT
    family = MONOMIAL
  []
  [T]
  []
[]

[Functions]
  [ic_func_gr0]
    type = ParsedFunction
    expression = '0.5*(1.0-tanh((x)/sqrt(2.0*2.0)))'
  []
  [ic_func_gr1]
    type = ParsedFunction
    expression = '0.5*(1.0+tanh((x)/sqrt(2.0*2.0)))'
  []
[]

[ICs]
  [gr0_IC]
    type = FunctionIC
    variable = gr0
    function = ic_func_gr0
  []
  [gr1_IC]
    type = FunctionIC
    variable = gr1
    function = ic_func_gr1
  []
  [wvy_IC]
    type = ConstantIC
    variable = wvy
    value = 2.7827
  []
  [wvo_IC]
    type = ConstantIC
    variable = wvo
    value = 2.7827
  []
  [T_IC]
    type = ConstantIC
    variable = T
    value = 1600
  []
[]

[BCs]
  [v_left]
    type = DirichletBC
    preset = true
    variable = V
    boundary = left
    value = 1e-2
  []
  [v_right]
    type = DirichletBC
    preset = true
    variable = V
    boundary = right
    value = 0
  []
  [gr0_left]
    type = DirichletBC
    preset = true
    variable = gr0
    boundary = left
    value = 0.5 #Grain boundary at left hand side of domain
  []
  [gr1_left]
    type = DirichletBC
    preset = true
    variable = gr1
    boundary = left
    value = 0.5 #Grain boundary at left hand side of domain
  []
  [wvo_right]
    type = DirichletBC
    preset = true
    variable = wvo
    boundary = right
    value = 2.7827
  []
  [wvy_right]
    type = DirichletBC
    preset = true
    variable = wvy
    boundary = right
    value = 2.7827
  []
[]

[Materials]
  # Free energy coefficients for parabolic curves
  [ks_cat]
    type = ParsedMaterial
    property_name = ks_cat
    coupled_variables = 'T'
    constant_names = 'a b Va'
    constant_expressions = '-0.0017 140.44 0.03726'
    expression = '(a*T + b) * Va^2'
  []
  [ks_an]
    type = ParsedMaterial
    property_name = ks_an
    coupled_variables = 'T'
    constant_names = 'a b Va'
    constant_expressions = '-0.0017 140.44 0.03726'
    expression = '(a*T + b) * Va^2'
  []
  [kv_cat]
    type = ParsedMaterial
    property_name = kv_cat
    material_property_names = 'ks_cat'
    expression = '10*ks_cat'
  []
  [kv_an]
    type = ParsedMaterial
    property_name = kv_an
    material_property_names = 'ks_cat'
    expression = '10*ks_cat'
  []
  # Diffusivity and mobilities
  [chiDy]
    type = GrandPotentialTensorMaterial
    f_name = chiDy
    diffusivity_name = Dvy
    solid_mobility = L
    void_mobility = Lv
    chi = chi_cat
    surface_energy = 6.24
    c = phi
    T = T
    D0 = 5.9e11
    GBmob0 = 1.60e12
    Q = 4.14
    Em = 4.25
    bulkindex = 1
    gbindex = 1
    surfindex = 1
  []
  [chiDo]
    type = GrandPotentialTensorMaterial
    f_name = chiDo
    diffusivity_name = Dvo
    solid_mobility = Lo
    void_mobility = Lvo
    chi = chi_an
    surface_energy = 6.24
    c = phi
    T = T
    D0 = 5.9e11
    GBmob0 = 1.60e12
    Q = 4.14
    Em = 4.25
    bulkindex = 1
    gbindex = 1
    surfindex = 1
  []
  # Everything else
  [ns_y_min]
    type = DerivativeParsedMaterial
    property_name = ns_y_min
    coupled_variables = 'gr0 gr1 T'
    constant_names = 'Ef_B Ef_GB   kB          Va_Y'
    constant_expressions = '4.37 4.37    8.617343e-5 0.03726'
    derivative_order = 2
    expression = 'bnds:=gr0^2 + gr1^2; Ef:=Ef_B + 4.0 * (Ef_GB - Ef_B) * (1.0 - bnds)^2;
              '
               '  exp(-Ef/kB/T) / Va_Y'
  []
  [ns_o_min]
    type = DerivativeParsedMaterial
    property_name = ns_o_min
    coupled_variables = 'gr0 gr1 T'
    constant_names = 'Ef_B Ef_GB  kB          Va_O'
    constant_expressions = '4.37 4.37   8.617343e-5 0.02484'
    derivative_order = 2
    expression = 'bnds:=gr0^2 + gr1^2; Ef:=Ef_B + 4.0 * (Ef_GB - Ef_B) * (1.0 - bnds)^2;
              '
               '  exp(-Ef/kB/T) / Va_O'
  []
  [sintering]
    type = ElectrochemicalSinteringMaterial
    chemical_potentials = 'wvy wvo'
    electric_potential = V
    void_op = phi
    Temperature = T
    surface_energy = 6.24
    grainboundary_energy = 5.18
    solid_energy_coefficients = 'kv_cat kv_cat'
    void_energy_coefficients = 'kv_cat kv_an'
    min_vacancy_concentrations_solid = 'ns_y_min ns_o_min'
    min_vacancy_concentrations_void = '26.837 40.256'
    defect_charges = '-3 2'
    solid_relative_permittivity = 30
    solid_energy_model = DILUTE
  []
  [density_chi_y]
    type = ElectrochemicalDefectMaterial
    chemical_potential = wvy
    void_op = phi
    Temperature = T
    electric_potential = V
    void_density_name = nv_cat
    solid_density_name = ns_cat
    chi_name = chi_cat
    void_energy_coefficient = kv_cat
    min_vacancy_concentration_solid = ns_y_min
    min_vacancy_concentration_void = 26.837
    solid_energy_model = DILUTE
    defect_charge = -3
    solid_relative_permittivity = 30
  []
  [density_chi_o]
    type = ElectrochemicalDefectMaterial
    chemical_potential = wvo
    void_op = phi
    Temperature = T
    electric_potential = V
    void_density_name = nv_an
    solid_density_name = ns_an
    chi_name = chi_an
    void_energy_coefficient = kv_an
    min_vacancy_concentration_solid = ns_o_min
    min_vacancy_concentration_void = 40.256
    solid_energy_model = DILUTE
    defect_charge = 2
    solid_relative_permittivity = 30
  []
  [permittivity]
    type = DerivativeParsedMaterial
    property_name = permittivity
    coupled_variables = 'phi'
    material_property_names = 'hs hv'
    constant_names = 'eps_rel_solid   eps_void_over_e'
    constant_expressions = '30              5.52e-2' #eps_void_over_e in 1/V/nm
    derivative_order = 2
    expression = '-hs * eps_rel_solid * eps_void_over_e - hv * eps_void_over_e'
  []
  [void_pre]
    type = DerivativeParsedMaterial
    property_name = void_pre
    material_property_names = 'hv'
    constant_names = 'Z_cat   Z_an nv_y_min nv_o_min'
    constant_expressions = '-3      2    26.837   40.256'
    derivative_order = 2
    expression = '-hv * (Z_cat * nv_y_min + Z_an * nv_o_min)'
  []
  [cat_mu_pre]
    type = DerivativeParsedMaterial
    property_name = cat_mu_pre
    material_property_names = 'hv kv_cat'
    constant_names = 'Z_cat'
    constant_expressions = '-3'
    derivative_order = 2
    expression = '-hv * Z_cat / kv_cat'
  []
  [an_mu_pre]
    type = DerivativeParsedMaterial
    property_name = an_mu_pre
    material_property_names = 'hv kv_an'
    constant_names = 'Z_an'
    constant_expressions = '2'
    derivative_order = 2
    expression = '-hv * Z_an / kv_an'
  []
  [cat_V_pre]
    type = DerivativeParsedMaterial
    property_name = cat_V_pre
    material_property_names = 'hv kv_cat'
    constant_names = 'Z_cat   v_scale e '
    constant_expressions = '-3      1       1'
    derivative_order = 2
    expression = 'hv * Z_cat^2 * e * v_scale / kv_cat'
  []
  [an_V_pre]
    type = DerivativeParsedMaterial
    property_name = an_V_pre
    material_property_names = 'hv kv_an'
    constant_names = 'Z_an    v_scale e '
    constant_expressions = '2       1       1'
    derivative_order = 2
    expression = 'hv * Z_an^2 * e * v_scale / kv_an'
  []
[]

#This action adds most kernels needed for grand potential model
[Modules]
  [PhaseField]
    [GrandPotential]
      switching_function_names = 'hv hs'
      anisotropic = 'true true'

      chemical_potentials = 'wvy wvo'
      mobilities = 'chiDy chiDo'
      susceptibilities = 'chi_cat chi_an'
      free_energies_w = 'nv_cat ns_cat nv_an ns_an'

      gamma_gr = gamma
      mobility_name_gr = L
      kappa_gr = kappa
      free_energies_gr = 'omegav omegas'

      additional_ops = 'phi'
      gamma_grxop = gamma
      mobility_name_op = Lv
      kappa_op = kappa
      free_energies_op = 'omegav omegas'
    []
  []
[]

[Kernels]
  [barrier_phi]
    type = ACBarrierFunction
    variable = phi
    v = 'gr0 gr1'
    gamma = gamma
    mob_name = Lv
  []
  [kappa_phi]
    type = ACKappaFunction
    variable = phi
    mob_name = Lv
    kappa_name = kappa
  []
  [Laplace]
    type = MatDiffusion
    variable = V
    diffusivity = permittivity
    args = 'phi'
  []
  [potential_void_constants]
    type = MaskedBodyForce
    variable = V
    coupled_variables = 'phi'
    mask = void_pre
  []
  [potential_cat_mu]
    type = MatReaction
    variable = V
    v = wvy
    mob_name = cat_mu_pre
  []
  [potential_an_mu]
    type = MatReaction
    variable = V
    v = wvo
    mob_name = an_mu_pre
  []
  [potential_cat_V]
    type = MatReaction
    variable = V
    mob_name = cat_V_pre
  []
  [potential_an_V]
    type = MatReaction
    variable = V
    mob_name = an_V_pre
  []
  [potential_solid_cat]
    type = MaskedExponential
    variable = V
    w = wvy
    T = T
    coupled_variables = 'phi gr0 gr1'
    mask = hs
    species_charge = -3
    n_eq = ns_y_min
  []
  [potential_solid_an]
    type = MaskedExponential
    variable = V
    w = wvo
    T = T
    coupled_variables = 'phi gr0 gr1'
    mask = hs
    species_charge = 2
    n_eq = ns_o_min
  []
[]

[AuxKernels]
  [bnds_aux]
    type = BndsCalcAux
    variable = bnds
    execute_on = 'initial timestep_end'
  []
  [negative_V]
    type = ParsedAux
    variable = negative_V
    coupled_variables = V
    expression = '-V'
  []
  [E_x]
    type = VariableGradientComponent
    variable = E_x
    gradient_variable = negative_V
    component = x
  []
  [E_y]
    type = VariableGradientComponent
    variable = E_y
    gradient_variable = negative_V
    component = y
  []
  [ns_cat_aux]
    type = MaterialRealAux
    variable = ns_cat_aux
    property = ns_cat
  []
  [ns_an_aux]
    type = MaterialRealAux
    variable = ns_an_aux
    property = ns_an
  []
[]

[Postprocessors]
  [ns_cat_total]
    type = ElementIntegralMaterialProperty
    mat_prop = ns_cat
  []
  [ns_an_total]
    type = ElementIntegralMaterialProperty
    mat_prop = ns_an
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -sub_pc_type -pc_asm_overlap -ksp_gmres_restart -sub_ksp_type'
  petsc_options_value = ' asm      lu           1               31                 preonly'
  nl_max_its = 40
  l_max_its = 30
  l_tol = 1e-4
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-13
  start_time = 0
  num_steps = 2
  automatic_scaling = true
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1
    optimal_iterations = 8
    iteration_window = 2
  []
[]

[Outputs]
  exodus = true
[]
