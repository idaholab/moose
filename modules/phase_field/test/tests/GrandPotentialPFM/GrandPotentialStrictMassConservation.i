[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 34
  ny = 34
  xmin = 0
  xmax = 340
  ymin = 0
  ymax = 340
[]

[GlobalParams]
  op_num = 2
  var_name_base = gr
  int_width = 20
[]

[Variables]
  [w]
  []
  [c]
  []
  [phi]
  []
  [PolycrystalVariables]
  []
[]

[AuxVariables]
  [bnds]
  []
  [T]
    order = CONSTANT
    family = MONOMIAL
    initial_condition = 1600
  []
[]

[ICs]
  [phi_IC]
    type = SpecifiedSmoothCircleIC
    variable = phi
    x_positions = '170 170'
    y_positions = ' 70 270'
    z_positions = '  0   0'
    radii = '100 100'
    invalue = 0
    outvalue = 1
  []
  [c_IC]
    type = SpecifiedSmoothCircleIC
    variable = c
    x_positions = '170 170'
    y_positions = ' 70 270'
    z_positions = '  0   0'
    radii = '100 100'
    invalue = 0
    outvalue = 1
  []
  [gr0_IC]
    type = SmoothCircleIC
    variable = gr0
    x1 = 170
    y1 = 70
    z1 = 0
    radius = 100
    invalue = 1
    outvalue = 0
  []
  [gr1_IC]
    type = SmoothCircleIC
    variable = gr1
    x1 = 170
    y1 = 270
    z1 = 0
    radius = 100
    invalue = 1
    outvalue = 0
  []
[]

[Materials]
  # Free energy coefficients for parabolic curves
  [./ks]
    type = ParsedMaterial
    property_name = ks
    coupled_variables = 'T'
    constant_names = 'a b'
    constant_expressions = '-0.0017 140.16'
    expression = 'a*T + b'
  [../]
  [./kv]
    type = ParsedMaterial
    property_name = kv
    material_property_names = 'ks'
    expression = '10*ks'
  [../]
  # Diffusivity and mobilities
  [chiD]
    type = GrandPotentialTensorMaterial
    f_name = chiD
    solid_mobility = L
    void_mobility = Lv
    chi = chi
    surface_energy = 6.24
    c = phi
    T = T
    D0 = 0.4366e9
    GBmob0 = 1.60e12
    Q = 4.14
    Em = 4.25
    bulkindex = 1
    gbindex = 1e6
    surfindex = 1e9
  []
  # Everything else
  [cv_eq]
    type = DerivativeParsedMaterial
    property_name = cv_eq
    coupled_variables = 'gr0 gr1 T'
    constant_names = 'Ef c_GB kB'
    constant_expressions = '4.37 0.1 8.617343e-5'
    derivative_order = 2
    expression = 'c_B:=exp(-Ef/kB/T); bnds:=gr0^2 + gr1^2;
                c_B + 4.0 * c_GB * (1.0 - bnds)^2'
  []
  [sintering]
    type = GrandPotentialSinteringMaterial
    chemical_potential = w
    void_op = phi
    Temperature = T
    surface_energy = 6.24
    grainboundary_energy = 5.18
    void_energy_coefficient = kv
    solid_energy_coefficient = ks
    solid_energy_model = PARABOLIC
    equilibrium_vacancy_concentration = cv_eq
  []
[]

[Modules]
  [PhaseField]
    [GrandPotential]
      switching_function_names = 'hv hs'
      anisotropic = 'true'
      chemical_potentials = 'w'
      mobilities = 'chiD'
      susceptibilities = 'chi'
      free_energies_w = 'rhov rhos'
      gamma_gr = gamma
      mobility_name_gr = L
      kappa_gr = kappa
      free_energies_gr = 'omegav omegas'
      additional_ops = 'phi'
      gamma_grxop = gamma
      mobility_name_op = Lv
      kappa_op = kappa
      free_energies_op = 'omegav omegas'
      mass_conservation = 'true'
      concentrations = 'c'
      hj_c_min = 'hv_c_min hs_c_min'
      hj_over_kVa = 'hv_over_kVa hs_over_kVa'
    []
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
  petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -pc_asm_overlap -ksp_gmres_restart -sub_ksp_type'
  petsc_options_value = 'gmres      asm      ilu          1               31                 preonly'
  nl_max_its = 30
  l_max_its = 30
  start_time = 0
  dt = 1e-4
  num_steps = 3
[]

[Outputs]
  exodus = true
[]
