# This is an example of implementation of the multi-phase, multi-order parameter
# grand potential based phase-field model described in Phys. Rev. E, 98, 023309
# (2018). It includes 3 phases with 1 grain of each phase. This example was used
# to generate the results shown in Fig. 3 of the paper.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 60
  xmin = -15
  xmax = 15
[]

[Variables]
  [w]
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
    type = FunctionIC
    variable = etaa0
    function = ic_func_etaa0
  []
  [IC_etab0]
    type = FunctionIC
    variable = etab0
    function = ic_func_etab0
  []
  [IC_etad0]
    type = ConstantIC
    variable = etad0
    value = 0.1
  []
  [IC_w]
    type = FunctionIC
    variable = w
    function = ic_func_w
  []
[]

[Functions]
  [ic_func_etaa0]
    type = ADParsedFunction
    value = '0.9*0.5*(1.0-tanh((x)/sqrt(2.0)))'
  []
  [ic_func_etab0]
    type = ADParsedFunction
    value = '0.9*0.5*(1.0+tanh((x)/sqrt(2.0)))'
  []
  [ic_func_w]
    type = ADParsedFunction
    value = 0
  []
[]

[Kernels]
# Order parameter eta_alpha0
  [ACa0_bulk]
    type = ADACGrGrMulti
    variable = etaa0
    v =           'etab0 etad0'
    gamma_names = 'gab   gad'
  []
  [ACa0_sw]
    type = ADACSwitching
    variable = etaa0
    Fj_names  = 'omegaa omegab omegad'
    hj_names  = 'ha     hb     hd'
  []
  [ACa0_int]
    type = ADACInterface
    variable = etaa0
    kappa_name = kappa
    variable_L = false
  []
  [ea0_dot]
    type = ADTimeDerivative
    variable = etaa0
  []
# Order parameter eta_beta0
  [ACb0_bulk]
    type = ADACGrGrMulti
    variable = etab0
    v =           'etaa0 etad0'
    gamma_names = 'gab   gbd'
  []
  [ACb0_sw]
    type = ADACSwitching
    variable = etab0
    Fj_names  = 'omegaa omegab omegad'
    hj_names  = 'ha     hb     hd'
  []
  [ACb0_int]
    type = ADACInterface
    variable = etab0
    kappa_name = kappa
    variable_L = false
  []
  [eb0_dot]
    type = ADTimeDerivative
    variable = etab0
  []
# Order parameter eta_delta0
  [ACd0_bulk]
    type = ADACGrGrMulti
    variable = etad0
    v =           'etaa0 etab0'
    gamma_names = 'gad   gbd'
  []
  [ACd0_sw]
    type = ADACSwitching
    variable = etad0
    Fj_names  = 'omegaa omegab omegad'
    hj_names  = 'ha     hb     hd'
  []
  [ACd0_int]
    type = ADACInterface
    variable = etad0
    kappa_name = kappa
    variable_L = false
  []
  [ed0_dot]
    type = ADTimeDerivative
    variable = etad0
  []
#Chemical potential
  [w_dot]
    type = ADSusceptibilityTimeDerivative
    variable = w
    f_name = chi
  []
  [Diffusion]
    type = ADMatDiffusion
    variable = w
    diffusivity = Dchi
  []
  [coupled_etaa0dot]
    type = ADCoupledSwitchingTimeDerivative
    variable = w
    v = etaa0
    Fj_names = 'rhoa rhob rhod'
    hj_names = 'ha   hb   hd'
    args = 'etaa0 etab0 etad0'
  []
  [coupled_etab0dot]
    type = ADCoupledSwitchingTimeDerivative
    variable = w
    v = etab0
    Fj_names = 'rhoa rhob rhod'
    hj_names = 'ha   hb   hd'
    args = 'etaa0 etab0 etad0'
  []
  [coupled_etad0dot]
    type = ADCoupledSwitchingTimeDerivative
    variable = w
    v = etad0
    Fj_names = 'rhoa rhob rhod'
    hj_names = 'ha   hb   hd'
    args = 'etaa0 etab0 etad0'
  []
[]

[Materials]
  [ha_test]
    type = ADSwitchingFunctionMultiPhaseMaterial
    h_name = ha
    all_etas = 'etaa0 etab0 etad0'
    phase_etas = 'etaa0'
  []
  [hb_test]
    type = ADSwitchingFunctionMultiPhaseMaterial
    h_name = hb
    all_etas = 'etaa0 etab0 etad0'
    phase_etas = 'etab0'
  []
  [hd_test]
    type = ADSwitchingFunctionMultiPhaseMaterial
    h_name = hd
    all_etas = 'etaa0 etab0 etad0'
    phase_etas = 'etad0'
  []
  [omegaa]
    type = ADDerivativeParsedMaterial
    args = 'w'
    f_name = omegaa
    material_property_names = 'Vm ka caeq'
    function = '-0.5*w^2/Vm^2/ka-w/Vm*caeq'
    derivative_order = 2
  []
  [omegab]
    type = ADDerivativeParsedMaterial
    args = 'w'
    f_name = omegab
    material_property_names = 'Vm kb cbeq'
    function = '-0.5*w^2/Vm^2/kb-w/Vm*cbeq'
    derivative_order = 2
  []
  [omegad]
    type = ADDerivativeParsedMaterial
    args = 'w'
    f_name = omegad
    material_property_names = 'Vm kd cdeq'
    function = '-0.5*w^2/Vm^2/kd-w/Vm*cdeq'
    derivative_order = 2
  []
  [rhoa]
    type = ADDerivativeParsedMaterial
    args = 'w'
    f_name = rhoa
    material_property_names = 'Vm ka caeq'
    function = 'w/Vm^2/ka + caeq/Vm'
    derivative_order = 2
  []
  [rhob]
    type = ADDerivativeParsedMaterial
    args = 'w'
    f_name = rhob
    material_property_names = 'Vm kb cbeq'
    function = 'w/Vm^2/kb + cbeq/Vm'
    derivative_order = 2
  []
  [rhod]
    type = ADDerivativeParsedMaterial
    args = 'w'
    f_name = rhod
    material_property_names = 'Vm kd cdeq'
    function = 'w/Vm^2/kd + cdeq/Vm'
    derivative_order = 2
  []
  [c]
    type = ADParsedMaterial
    material_property_names = 'Vm rhoa rhob rhod ha hb hd'
    function = 'Vm * (ha * rhoa + hb * rhob + hd * rhod)'
    f_name = c
  []
  [const]
    type = ADGenericConstantMaterial
    prop_names =  'kappa_c  kappa   L   D    Vm   ka    caeq kb    cbeq  kd    cdeq  gab gad gbd  mu  tgrad_corr_mult'
    prop_values = '0        1       1.0 1.0  1.0  10.0  0.1  10.0  0.9   10.0  0.5   1.5 1.5 1.5  1.0 0.0'
  []
  [Mobility]
    type = ADDerivativeParsedMaterial
    f_name = Dchi
    material_property_names = 'D chi'
    function = 'D*chi'
    derivative_order = 2
  []
  [chi]
    type = ADDerivativeParsedMaterial
    f_name = chi
    material_property_names = 'Vm ha(etaa0,etab0,etad0) ka hb(etaa0,etab0,etad0) kb hd(etaa0,etab0,etad0) kd'
    function = '(ha/ka + hb/kb + hd/kd) / Vm^2'
    args = 'etaa0 etab0 etad0'
    derivative_order = 2
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[VectorPostprocessors]
  [etaa0]
    type = LineValueSampler
    variable = etaa0
    start_point = '-15 0 0'
    end_point = '15 0 0'
    num_points = 61
    sort_by = x
    execute_on = 'initial timestep_end final'
  []
  [etab0]
    type = LineValueSampler
    variable = etab0
    start_point = '-15 0 0'
    end_point = '15 0 0'
    num_points = 61
    sort_by = x
    execute_on = 'initial timestep_end final'
  []
  [etad0]
    type = LineValueSampler
    variable = etad0
    start_point = '-15 0 0'
    end_point = '15 0 0'
    num_points = 61
    sort_by = x
    execute_on = 'initial timestep_end final'
  []
[]

[Executioner]
  type = Transient
  nl_max_its = 15
  scheme = bdf2
  solve_type = NEWTON
  petsc_options_iname = -pc_type
  petsc_options_value = lu
  l_max_its = 15
  l_tol = 1.0e-3
  nl_rel_tol = 1.0e-8
  start_time = 0.0
  num_steps = 20
  nl_abs_tol = 1e-10
  dt = 1.0
[]

[Outputs]
  [exodus]
    type = Exodus
    execute_on = 'initial timestep_end final'
    interval = 1
  []
  [csv]
    type = CSV
    execute_on = 'initial timestep_end final'
    interval = 1
  []
[]
