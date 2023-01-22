#input file to test the GrandPotentialSinteringMaterial using the parabolic energy profile

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 17
  ny = 10
  xmin = 0
  xmax = 660
  ymin = 0
  ymax = 380
[]

[GlobalParams]
  op_num = 2
  var_name_base = gr
  int_width = 40
[]

[Variables]
  [./w]
    [./InitialCondition]
      type = FunctionIC
      variable = w
      function = f_w
    [../]
  [../]
  [./phi]
  [../]
  [./PolycrystalVariables]
  [../]
[]

[AuxVariables]
  [./T]
    order = CONSTANT
    family = MONOMIAL
    [./InitialCondition]
      type = FunctionIC
      variable = T
      function = f_T
    [../]
  [../]
[]

[ICs]
  [./phi_IC]
    type = SpecifiedSmoothCircleIC
    variable = phi
    x_positions = '190 470'
    y_positions = '190 190'
    z_positions = '  0   0'
    radii = '150 150'
    invalue = 0
    outvalue = 1
  [../]
  [./gr0_IC]
    type = SmoothCircleIC
    variable = gr0
    x1 = 190
    y1 = 190
    z1 = 0
    radius = 150
    invalue = 1
    outvalue = 0
  [../]
  [./gr1_IC]
    type = SmoothCircleIC
    variable = gr1
    x1 = 470
    y1 = 190
    z1 = 0
    radius = 150
    invalue = 1
    outvalue = 0
  [../]
[]

[Functions]
  [./f_T]
    type = ConstantFunction
    value = 1600
  [../]
  [./f_w]
    type = ParsedFunction
    expression = '1.515e-7 * x'
  [../]
[]

[Materials]
  # Free energy coefficients for parabolic curve
  [./ks]
    type = ParsedMaterial
    property_name = ks
    coupled_variables = 'T'
    constant_names = 'a b'
    constant_expressions = '-0.0025 157.16'
    expression = 'a*T + b'
  [../]
  [./kv]
    type = ParsedMaterial
    property_name = kv
    material_property_names = 'ks'
    expression = '10 * ks'
  [../]
  # Diffusivity and mobilities
  [./chiD]
    type = GrandPotentialTensorMaterial
    f_name = chiD
    solid_mobility = L
    void_mobility = Lv
    chi = chi
    surface_energy = 19.7
    c = phi
    T = T
    D0 = 2.0e11
    GBmob0 = 1.4759e9
    Q = 2.77
    Em = 2.40
    bulkindex = 1
    gbindex = 20
    surfindex = 100
  [../]
  # Equilibrium vacancy concentration
  [./cs_eq]
    type = DerivativeParsedMaterial
    property_name = cs_eq
    coupled_variables = 'gr0 gr1 T'
    constant_names = 'Ef Egb kB'
    constant_expressions = '2.69 2.1 8.617343e-5'
    expression = 'bnds:=gr0^2 + gr1^2; cb:=exp(-Ef/kB/T); cgb:=exp(-(Ef-Egb)/kB/T);
                cb + 4.0*(cgb-cb)*(1.0 - bnds)^2'
  [../]
  # Everything else
  [./sintering]
    type = GrandPotentialSinteringMaterial
    chemical_potential = w
    void_op = phi
    Temperature = T
    surface_energy = 19.7
    grainboundary_energy = 9.86
    void_energy_coefficient = kv
    equilibrium_vacancy_concentration = cs_eq
    solid_energy_model = PARABOLIC
    outputs = exodus
  [../]

  # Concentration is only meant for output
  [./c]
    type = ParsedMaterial
    property_name = c
    material_property_names = 'hs rhos hv rhov'
    constant_names = 'Va'
    constant_expressions = '0.04092'
    expression = 'Va*(hs*rhos + hv*rhov)'
    outputs = exodus
  [../]
[]

[Kernels]
  [./dt_gr0]
    type = TimeDerivative
    variable = gr0
  [../]
  [./dt_gr1]
    type = TimeDerivative
    variable = gr1
  [../]
  [./dt_phi]
    type = TimeDerivative
    variable = phi
  [../]
  [./dt_w]
    type = TimeDerivative
    variable = w
  [../]
[]

[AuxKernels]
  [./T_aux]
    type = FunctionAux
    variable = T
    function = f_T
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = JFNK
  dt = 1
  num_steps = 2
  nl_abs_tol = 1e-10
[]

[Outputs]
  exodus = true
[]
