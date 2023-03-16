# This is an example of implementation of the multi-phase, multi-order parameter
# grand potential based phase-field model described in Phys. Rev. E, 98, 023309
# (2019). It includes 3 phases with 1 grain of each phase.
# This is a revised version of the model that eliminates small variations in mass
# that have been observed with the original formulation. In this version, rather
# than evolving the chemical potential as a field variable, we evolve the composition
# field using a normal Cahn-Hilliard equation, then relate chemical potential to
# composition using Eq. (22) from the paper (this relationship is derived from the
# grand potential functional and is valid only for parabolic free energies).

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 60
  ny = 60
  xmin = -15
  xmax = 15
  ymin = -15
  ymax = 15
[]

[Variables]
  [w]
  []
  [c]
  []
  [etaa0]
  []
  [etab0]
  []
  [etad0]
  []
[]

[ICs]
  [IC_etaa0]
    type = BoundingBoxIC
    variable = etaa0
    x1 = -10
    y1 = -10
    x2 = 10
    y2 = 10
    inside = 1.0
    outside = 0.0
  []
  [IC_etad0]
    type = BoundingBoxIC
    variable = etad0
    x1 = -10
    y1 = -10
    x2 = 10
    y2 = 10
    inside = 0.0
    outside = 1.0
  []
  [IC_c]
    type = BoundingBoxIC
    variable = c
    x1 = -10
    y1 = -10
    x2 = 10
    y2 = 10
    inside = 0.1
    outside = 0.5
  []
  [IC_w]
    type = FunctionIC
    variable = w
    function = ic_func_w
  []
[]

[Functions]
  [ic_func_w]
    type = ConstantFunction
    value = 0
  []
[]

[Kernels]
  # Order parameter eta_alpha0
  [ACa0_bulk]
    type = ACGrGrMulti
    variable = etaa0
    v = 'etab0 etad0'
    gamma_names = 'gab   gad'
  []
  [ACa0_sw]
    type = ACSwitching
    variable = etaa0
    Fj_names = 'omegaa omegab omegad'
    hj_names = 'ha     hb     hd'
    coupled_variables = 'etab0 etad0 w'
  []
  [ACa0_int]
    type = ACInterface
    variable = etaa0
    kappa_name = kappa
  []
  [ea0_dot]
    type = TimeDerivative
    variable = etaa0
  []
  # Order parameter eta_beta0
  [ACb0_bulk]
    type = ACGrGrMulti
    variable = etab0
    v = 'etaa0 etad0'
    gamma_names = 'gab   gbd'
  []
  [ACb0_sw]
    type = ACSwitching
    variable = etab0
    Fj_names = 'omegaa omegab omegad'
    hj_names = 'ha     hb     hd'
    coupled_variables = 'etaa0 etad0 w'
  []
  [ACb0_int]
    type = ACInterface
    variable = etab0
    kappa_name = kappa
  []
  [eb0_dot]
    type = TimeDerivative
    variable = etab0
  []
  # Order parameter eta_delta0
  [ACd0_bulk]
    type = ACGrGrMulti
    variable = etad0
    v = 'etaa0 etab0'
    gamma_names = 'gad   gbd'
  []
  [ACd0_sw]
    type = ACSwitching
    variable = etad0
    Fj_names = 'omegaa omegab omegad'
    hj_names = 'ha     hb     hd'
    coupled_variables = 'etaa0 etab0 w'
  []
  [ACd0_int]
    type = ACInterface
    variable = etad0
    kappa_name = kappa
  []
  [ed0_dot]
    type = TimeDerivative
    variable = etad0
  []
  #Concentration
  [c_dot]
    type = TimeDerivative
    variable = c
  []
  [Diffusion]
    type = MatDiffusion
    variable = c
    v = w
    diffusivity = DchiVm
    args = ''
  []
  #The following relate chemical potential to composition using Eq. (22)
  [w_rxn]
    type = MatReaction
    variable = w
    v = c
    mob_name = -1
  []
  [ca_rxn]
    type = MatReaction
    variable = w
    mob_name = 'hoverk_a'
    args = 'etaa0 etab0 etad0'
  []
  [ca_bodyforce]
    type = MaskedBodyForce
    variable = w
    mask = ha
    coupled_variables = 'etaa0 etab0 etad0'
    value = 0.1 #caeq
  []
  [cb_rxn]
    type = MatReaction
    variable = w
    mob_name = 'hoverk_b'
    args = 'etaa0 etab0 etad0'
  []
  [cb_bodyforce]
    type = MaskedBodyForce
    variable = w
    mask = hb
    coupled_variables = 'etaa0 etab0 etad0'
    value = 0.9 #cbeq
  []
  [cd_rxn]
    type = MatReaction
    variable = w
    mob_name = 'hoverk_d'
    args = 'etaa0 etab0 etad0'
  []
  [cd_bodyforce]
    type = MaskedBodyForce
    variable = w
    mask = hd
    coupled_variables = 'etaa0 etab0 etad0'
    value = 0.5 #cdeq
  []
[]

[Materials]
  [ha_test]
    type = SwitchingFunctionMultiPhaseMaterial
    h_name = ha
    all_etas = 'etaa0 etab0 etad0'
    phase_etas = 'etaa0'
  []
  [hb_test]
    type = SwitchingFunctionMultiPhaseMaterial
    h_name = hb
    all_etas = 'etaa0 etab0 etad0'
    phase_etas = 'etab0'
  []
  [hd_test]
    type = SwitchingFunctionMultiPhaseMaterial
    h_name = hd
    all_etas = 'etaa0 etab0 etad0'
    phase_etas = 'etad0'
  []
  [omegaa]
    type = DerivativeParsedMaterial
    coupled_variables = 'w'
    property_name = omegaa
    material_property_names = 'Vm ka caeq'
    expression = '-0.5*w^2/Vm^2/ka-w/Vm*caeq'
    derivative_order = 2
  []
  [omegab]
    type = DerivativeParsedMaterial
    coupled_variables = 'w'
    property_name = omegab
    material_property_names = 'Vm kb cbeq'
    expression = '-0.5*w^2/Vm^2/kb-w/Vm*cbeq'
    derivative_order = 2
  []
  [omegad]
    type = DerivativeParsedMaterial
    coupled_variables = 'w'
    property_name = omegad
    material_property_names = 'Vm kd cdeq'
    expression = '-0.5*w^2/Vm^2/kd-w/Vm*cdeq'
    derivative_order = 2
  []
  [rhoa]
    type = DerivativeParsedMaterial
    coupled_variables = 'w'
    property_name = rhoa
    material_property_names = 'Vm ka caeq'
    expression = 'w/Vm^2/ka + caeq/Vm'
    derivative_order = 2
  []
  [rhob]
    type = DerivativeParsedMaterial
    coupled_variables = 'w'
    property_name = rhob
    material_property_names = 'Vm kb cbeq'
    expression = 'w/Vm^2/kb + cbeq/Vm'
    derivative_order = 2
  []
  [rhod]
    type = DerivativeParsedMaterial
    coupled_variables = 'w'
    property_name = rhod
    material_property_names = 'Vm kd cdeq'
    expression = 'w/Vm^2/kd + cdeq/Vm'
    derivative_order = 2
  []
  [const]
    type = GenericConstantMaterial
    prop_names = 'kappa_c  kappa   L   D    Vm   ka    caeq kb    cbeq  kd    cdeq  gab gad gbd  mu  tgrad_corr_mult'
    prop_values = '0        1       1.0 1.0  1.0  10.0  0.1  10.0  0.9   10.0  0.5   1.5 1.5 1.5  1.0 0.0'
  []
  [Mobility]
    type = DerivativeParsedMaterial
    property_name = DchiVm
    material_property_names = 'D chi Vm' #Factor of Vm is needed to evolve c instead of rho
    expression = 'D*chi*Vm'
    derivative_order = 2
  []
  [chi]
    type = DerivativeParsedMaterial
    property_name = chi
    material_property_names = 'Vm ha(etaa0,etab0,etad0) ka hb(etaa0,etab0,etad0) kb hd(etaa0,etab0,etad0) kd'
    expression = '(ha/ka + hb/kb + hd/kd) / Vm^2'
    coupled_variables = 'etaa0 etab0 etad0'
    derivative_order = 2
  []
  [hoverk_a]
    type = DerivativeParsedMaterial
    material_property_names = 'ha(etaa0,etab0,etad0) Vm ka'
    property_name = hoverk_a
    expression = 'ha / Vm / ka'
  []
  [hoverk_b]
    type = DerivativeParsedMaterial
    material_property_names = 'hb(etaa0,etab0,etad0) Vm kb'
    property_name = hoverk_b
    expression = 'hb / Vm / kb'
  []
  [hoverk_d]
    type = DerivativeParsedMaterial
    material_property_names = 'hd(etaa0,etab0,etad0) Vm kd'
    property_name = hoverk_d
    expression = 'hd / Vm / kd'
  []
[]

[Postprocessors]
  [c_total]
    type = ElementIntegralVariablePostprocessor
    variable = c
  []
[]

[Executioner]
  type = Transient
  nl_max_its = 15
  scheme = bdf2
  solve_type = NEWTON
  petsc_options_iname = -pc_type
  petsc_options_value = asm
  l_max_its = 15
  l_tol = 1.0e-3
  nl_rel_tol = 1.0e-8
  start_time = 0.0
  num_steps = 20
  nl_abs_tol = 1e-10
  dt = 1.0
[]

[Outputs]
  csv = true
  exodus = true
[]
