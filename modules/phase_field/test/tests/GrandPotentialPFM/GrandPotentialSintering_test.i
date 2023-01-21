#input file to test the materials GrandPotentialTensorMaterial

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 17
  ny = 17
  xmin = 0
  xmax = 680
  ymin = 0
  ymax = 680
  uniform_refine = 1
[]

[GlobalParams]
  op_num = 4
  var_name_base = gr
  int_width = 40
[]

[Variables]
  [./w]
  [../]
  [./phi]
  [../]
  [./PolycrystalVariables]
  [../]
[]

[AuxVariables]
  [./bnds]
  [../]
  [./T]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./F_loc]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[ICs]
  [./phi_IC]
    type = SpecifiedSmoothCircleIC
    variable = phi
    x_positions = '190 490 190 490'
    y_positions = '190 190 490 490'
    z_positions = '  0   0   0   0'
    radii = '150 150 150 150'
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
    x1 = 490
    y1 = 190
    z1 = 0
    radius = 150
    invalue = 1
    outvalue = 0
  [../]
  [./gr2_IC]
    type = SmoothCircleIC
    variable = gr2
    x1 = 190
    y1 = 490
    z1 = 0
    radius = 150
    invalue = 1
    outvalue = 0
  [../]
  [./gr3_IC]
    type = SmoothCircleIC
    variable = gr3
    x1 = 490
    y1 = 490
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
[]

[Materials]
  # Free energy coefficients for parabolic curves
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
    expression = '10*ks'
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
    outputs = exodus
  [../]
  # Equilibrium vacancy concentration
  [./cs_eq]
    type = DerivativeParsedMaterial
    property_name = cs_eq
    coupled_variables = 'gr0 gr1 gr2 gr3 T'
    constant_names = 'Ef c_GB kB'
    constant_expressions = '2.69 0.189 8.617343e-5'
    expression = 'bnds:=gr0^2 + gr1^2 + gr2^2 + gr3^2; exp(-Ef/kB/T) + 4.0 * c_GB * (1 - bnds)^2'
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
    solid_energy_coefficient = ks
    equilibrium_vacancy_concentration = cs_eq
    solid_energy_model = PARABOLIC
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
  [./f_bulk]
    type = ParsedMaterial
    property_name = f_bulk
    coupled_variables = 'phi gr0 gr1 gr2 gr3'
    material_property_names = 'mu gamma'
    expression = 'mu*(phi^4/4-phi^2/2 + gr0^4/4-gr0^2/2 + gr1^4/4-gr1^2/2
                  + gr2^4/4-gr2^2/2 + gr3^4/4-gr3^2/2
                  + gamma*(phi^2*(gr0^2+gr1^2+gr2^2+gr3^2) + gr0^2*(gr1^2+gr2^2+gr3^2)
                  + gr1^2*(gr2^2 + gr3^2) + gr2^2*gr3^2) + 0.25)'
    outputs = exodus
  [../]
  [./f_switch]
    type = ParsedMaterial
    property_name = f_switch
    coupled_variables = 'w'
    material_property_names = 'chi'
    expression = '0.5*w^2*chi'
    outputs = exodus
  [../]
  [./f0]
    type = ParsedMaterial
    property_name = f0
    material_property_names = 'f_bulk f_switch'
    expression = 'f_bulk + f_switch'
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
  [./dt_gr2]
    type = TimeDerivative
    variable = gr2
  [../]
  [./dt_gr3]
    type = TimeDerivative
    variable = gr3
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
  [./bnds_aux]
    type = BndsCalcAux
    variable = bnds
    execute_on = 'initial timestep_end'
  [../]
  [./T_aux]
    type = FunctionAux
    variable = T
    function = f_T
  [../]
  [./F_aux]
    type = TotalFreeEnergy
    variable = F_loc
    f_name = f0
    interfacial_vars = 'phi gr0 gr1 gr2 gr3'
    kappa_names = 'kappa kappa kappa kappa kappa'
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = JFNK
  dt = 1
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
