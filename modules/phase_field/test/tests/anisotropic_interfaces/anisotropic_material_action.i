[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 28
  ny = 28
  xmin = -7
  xmax = 7
  ymin = -7
  ymax = 7
[]

[GlobalParams]
  radius = 2.0
  int_width = 1.0
  x1 = 0.0
  y1 = 0.0
  derivative_order = 2
[]

[Variables]
  [w]
  []
  [etaa0]
  []
  [etab0]
  []
  [T]
  []
[]

[AuxVariables]
  [bnds]
  []
[]

[AuxKernels]
  [bnds]
    type = BndsCalcAux
    variable = bnds
    v = 'etaa0 etab0'
  []
[]

[ICs]
  [w]
    type = SmoothCircleIC
    variable = w
    # note w = A*(c-cleq), A = 1.0, cleq = 0.0 ,i.e., w = c (in the matrix/liquid phase)
    outvalue = -4.0
    invalue = 0.0
  []
  [etaa0]
    type = SmoothCircleIC
    variable = etaa0
    #Solid phase
    outvalue = 0.0
    invalue = 1.0
  []
  [etab0]
    type = SmoothCircleIC
    variable = etab0
    #Liquid phase
    outvalue = 1.0
    invalue = 0.0
  []
[]

[Kernels]
  # Order parameter eta_alpha0
  [ACa0_bulk]
    type = ACGrGrMulti
    variable = etaa0
    v = 'etab0'
    gamma_names = 'gab'
  []
  [ACa0_sw]
    type = ACSwitching
    variable = etaa0
    Fj_names = 'omegaa omegab'
    hj_names = 'ha     hb'
    coupled_variables = 'etab0 w T'
  []
  [ACa0_int1]
    type = ACInterface2DMultiPhase1
    variable = etaa0
    etas = 'etab0'
    kappa_name = kappa_etaa0_etab0
    dkappadgrad_etaa_name = dkappadgrad_etaa0_etab0
    d2kappadgrad_etaa_name = d2kappadgrad_etaa0_etab0
  []
  [ACa0_int2]
    type = ACInterface2DMultiPhase2
    variable = etaa0
    kappa_name = kappa_etaa0_etab0
    dkappadgrad_etaa_name = dkappadgrad_etaa0_etab0
  []
  [ea0_dot]
    type = TimeDerivative
    variable = etaa0
  []
  # Order parameter eta_beta0
  [ACb0_bulk]
    type = ACGrGrMulti
    variable = etab0
    v = 'etaa0'
    gamma_names = 'gab'
  []
  [ACb0_sw]
    type = ACSwitching
    variable = etab0
    Fj_names = 'omegaa omegab'
    hj_names = 'ha     hb'
    coupled_variables = 'etaa0 w T'
  []
  [ACb0_int1]
    type = ACInterface2DMultiPhase1
    variable = etab0
    etas = 'etaa0'
    kappa_name = kappa_etab0_etaa0
    dkappadgrad_etaa_name = dkappadgrad_etab0_etaa0
    d2kappadgrad_etaa_name = d2kappadgrad_etab0_etaa0
  []
  [ACb0_int2]
    type = ACInterface2DMultiPhase2
    variable = etab0
    kappa_name = kappa_etab0_etaa0
    dkappadgrad_etaa_name = dkappadgrad_etab0_etaa0
  []
  [eb0_dot]
    type = TimeDerivative
    variable = etab0
  []
  #Chemical potential
  [w_dot]
    type = SusceptibilityTimeDerivative
    variable = w
    f_name = chi
  []
  [Diffusion]
    type = MatDiffusion
    variable = w
    diffusivity = Dchi
  []
  [coupled_etaa0dot]
    type = CoupledSwitchingTimeDerivative
    variable = w
    v = etaa0
    Fj_names = 'rhoa rhob'
    hj_names = 'ha   hb'
    coupled_variables = 'etaa0 etab0'
  []
  [coupled_etab0dot]
    type = CoupledSwitchingTimeDerivative
    variable = w
    v = etab0
    Fj_names = 'rhoa rhob'
    hj_names = 'ha   hb'
    coupled_variables = 'etaa0 etab0'
  []
  [T_dot]
    type = TimeDerivative
    variable = T
  []
  [CoefDiffusion]
    type = Diffusion
    variable = T
  []
  [etaa0_dot_T]
    type = CoefCoupledTimeDerivative
    variable = T
    v = etaa0
    coef = -5.0
  []
[]

[Materials]
  [ha]
    type = SwitchingFunctionMultiPhaseMaterial
    h_name = ha
    all_etas = 'etaa0 etab0'
    phase_etas = 'etaa0'
  []
  [hb]
    type = SwitchingFunctionMultiPhaseMaterial
    h_name = hb
    all_etas = 'etaa0 etab0'
    phase_etas = 'etab0'
  []
  [omegaa]
    type = DerivativeParsedMaterial
    coupled_variables = 'w'
    property_name = omegaa
    material_property_names = 'Vm ka caeq'
    expression = '-0.5*w^2/Vm^2/ka-w/Vm*caeq'
  []
  [omegab]
    type = DerivativeParsedMaterial
    coupled_variables = 'w T'
    property_name = omegab
    material_property_names = 'Vm kb cbeq S Tm'
    expression = '-0.5*w^2/Vm^2/kb-w/Vm*cbeq-S*(T-Tm)'
  []
  [rhoa]
    type = DerivativeParsedMaterial
    coupled_variables = 'w'
    property_name = rhoa
    material_property_names = 'Vm ka caeq'
    expression = 'w/Vm^2/ka + caeq/Vm'
  []
  [rhob]
    type = DerivativeParsedMaterial
    coupled_variables = 'w'
    property_name = rhob
    material_property_names = 'Vm kb cbeq'
    expression = 'w/Vm^2/kb + cbeq/Vm'
  []
  [const]
    type = GenericConstantMaterial
    prop_names = 'L     D    chi  Vm   ka    caeq kb    cbeq  gab mu   S   Tm'
    prop_values = '33.33 1.0  0.1  1.0  10.0  0.1  10.0  0.9   4.5 10.0 1.0 5.0'
  []
  [Mobility]
    type = ParsedMaterial
    property_name = Dchi
    material_property_names = 'D chi'
    expression = 'D*chi'
  []
[]

[Modules]
  [PhaseField]
    [AnisotropyInterface]
      etas = 'etaa0 etab0'
      kappa_bar = 1.0
      anisotropy_strength = 0.05
      reference_angle = '0 0'
      kappa_name = kappa
      dkappadgrad_eta_name = dkappadgrad
      d2kappadgrad_eta_name = d2kappadgrad
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
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre    boomeramg      31'
  l_tol = 1.0e-3
  l_max_its = 30
  nl_max_its = 15
  nl_rel_tol = 1.0e-8
  nl_abs_tol = 1e-10
  end_time = 2.0
  dt = 0.0001
  num_steps = 5
[]

[Outputs]
  exodus = true
[]
