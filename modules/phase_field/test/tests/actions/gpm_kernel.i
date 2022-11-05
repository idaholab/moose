[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 100
  xmin = 0
  xmax = 300
[]

[GlobalParams]
  op_num = 1
  var_name_base = eta
[]

[Variables]
  [./w]
  [../]
  [./phi]
  [../]
  [./eta0]
  [../]
[]

[AuxVariables]
  [./bnds]
  [../]
[]

[ICs]
  [./IC_w]
    type = BoundingBoxIC
    variable = w
    x1 = 150
    x2 = 300
    y1 = 0
    y2 = 0
    inside = 0.1
    outside = 0
  [../]
  [./IC_phi]
    type = BoundingBoxIC
    variable = phi
    x1 = 0
    x2 = 150
    y1 = 0
    y2 = 0
    inside = 1
    outside = 0
  [../]
  [./IC_eta0]
    type = BoundingBoxIC
    variable = eta0
    x1 = 150
    x2 = 300
    y1 = 0
    y2 = 0
    inside = 1
    outside = 0
  [../]
[]

[AuxKernels]
  [./bnds_aux]
    type = BndsCalcAux
    variable = bnds
  [../]
[]

[Modules]
  [./PhaseField]
    [./GrandPotential]
      switching_function_names = 'hb hm'

      chemical_potentials = 'w'
      anisotropic = 'false'
      mobilities = 'chiD'
      susceptibilities = 'chi'
      free_energies_w = 'rhob rhom'

      gamma_gr = gamma
      mobility_name_gr = L
      kappa_gr = kappa
      free_energies_gr = 'omegab omegam'

      additional_ops = 'phi'
      gamma_grxop = gamma
      mobility_name_op = L_phi
      kappa_op = kappa
      free_energies_op = 'omegab omegam'
    [../]
  [../]
[]

[Materials]
  #REFERENCES
  [./constants]
    type = GenericConstantMaterial
    prop_names =  'Va      cb_eq cm_eq kb   km  mu  gamma L      L_phi  kappa  kB'
    prop_values = '0.04092 1.0   1e-5  1400 140 1.5 1.5   5.3e+3 2.3e+4 295.85 8.6173324e-5'
  [../]
  #SWITCHING FUNCTIONS
  [./switchb]
    type = SwitchingFunctionMultiPhaseMaterial
    h_name = hb
    all_etas = 'phi eta0'
    phase_etas = 'phi'
  [../]
  [./switchm]
    type = SwitchingFunctionMultiPhaseMaterial
    h_name = hm
    all_etas = 'phi eta0'
    phase_etas = 'eta0'
  [../]
  [./omegab]
    type = DerivativeParsedMaterial
    property_name = omegab
    coupled_variables = 'w phi'
    material_property_names = 'Va kb cb_eq'
    expression = '-0.5*w^2/Va^2/kb - w/Va*cb_eq'
    derivative_order = 2
  [../]
  [./omegam]
    type = DerivativeParsedMaterial
    property_name = omegam
    coupled_variables = 'w eta0'
    material_property_names = 'Va km cm_eq'
    expression = '-0.5*w^2/Va^2/km - w/Va*cm_eq'
    derivative_order = 2
  [../]
  [./chi]
    type = DerivativeParsedMaterial
    property_name = chi
    coupled_variables = 'w'
    material_property_names = 'Va hb hm kb km'
    expression = '(hm/km + hb/kb)/Va^2'
    derivative_order = 2
  [../]
  #DENSITIES/CONCENTRATION
  [./rhob]
    type = DerivativeParsedMaterial
    property_name = rhob
    coupled_variables = 'w'
    material_property_names = 'Va kb cb_eq'
    expression = 'w/Va^2/kb + cb_eq/Va'
    derivative_order = 1
  [../]
  [./rhom]
    type = DerivativeParsedMaterial
    property_name = rhom
    coupled_variables = 'w eta0'
    material_property_names = 'Va km cm_eq(eta0)'
    expression = 'w/Va^2/km + cm_eq/Va'
    derivative_order = 1
  [../]
  [./concentration]
    type = ParsedMaterial
    property_name = c
    material_property_names = 'rhom hm rhob hb Va'
    expression = 'Va*(hm*rhom + hb*rhob)'
    outputs = exodus
  [../]
  [./mobility]
    type = DerivativeParsedMaterial
    material_property_names = 'chi kB'
    constant_names = 'T Em D0'
    constant_expressions = '1400 2.4 1.25e2'
    property_name = chiD
    expression = 'chi*D0*exp(-Em/kB/T)'
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -sub_pc_type -pc_asm_overlap -ksp_gmres_restart -sub_ksp_type'
  petsc_options_value = ' asm      lu           1               31                 preonly'
  nl_max_its = 20
  l_max_its = 30
  l_tol = 1e-4
  nl_rel_tol = 1e-7
  nl_abs_tol = 1e-7
  start_time = 0
  dt = 2e-5
  num_steps = 3
[]

[Outputs]
  exodus = true
[]
