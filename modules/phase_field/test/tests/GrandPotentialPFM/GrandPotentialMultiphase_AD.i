[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  xmin = -20
  xmax = 20
  ymin = -20
  ymax = 20
[]

[GlobalParams]
  op_num = 2
  var_name_base = etab
[]

[Variables]
  [w]
  []
  [etaa0]
  []
  [etab0]
  []
  [etab1]
  []
[]

[AuxVariables]
  [bnds]
    order = FIRST
    family = LAGRANGE
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
  [IC_etab1]
    type = FunctionIC
    variable = etab1
    function = ic_func_etab1
  []
  [IC_w]
    type = ConstantIC
    value = -0.05
    variable = w
  []
[]

[Functions]
  [ic_func_etaa0]
    type = ADParsedFunction
    value = 'r:=sqrt(x^2+y^2);0.5*(1.0-tanh((r-10.0)/sqrt(2.0)))'
  []
  [ic_func_etab0]
    type = ADParsedFunction
    value = 'r:=sqrt(x^2+y^2);0.5*(1.0+tanh((r-10)/sqrt(2.0)))*0.5*(1.0+tanh((y)/sqrt(2.0)))'
  []
  [ic_func_etab1]
    type = ADParsedFunction
    value = 'r:=sqrt(x^2+y^2);0.5*(1.0+tanh((r-10)/sqrt(2.0)))*0.5*(1.0-tanh((y)/sqrt(2.0)))'
  []
[]


[BCs]
[]

[Kernels]
# Order parameter eta_alpha0
  [ACa0_bulk]
    type = ADACGrGrMulti
    variable = etaa0
    v =           'etab0 etab1'
    gamma_names = 'gab   gab'
  []
  [ACa0_sw]
    type = ADACSwitching
    variable = etaa0
    Fj_names  = 'omegaa omegab'
    hj_names  = 'ha     hb'
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
    v =           'etaa0 etab1'
    gamma_names = 'gab   gbb'
  []
  [ACb0_sw]
    type = ADACSwitching
    variable = etab0
    Fj_names  = 'omegaa omegab'
    hj_names  = 'ha     hb'
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
# Order parameter eta_beta1
  [ACb1_bulk]
    type = ADACGrGrMulti
    variable = etab1
    v =           'etaa0 etab0'
    gamma_names = 'gab   gbb'
  []
  [ACb1_sw]
    type = ADACSwitching
    variable = etab1
    Fj_names  = 'omegaa omegab'
    hj_names  = 'ha     hb'
  []
  [ACb1_int]
    type = ADACInterface
    variable = etab1
    kappa_name = kappa
    variable_L = false
  []
  [eb1_dot]
    type = ADTimeDerivative
    variable = etab1
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
    Fj_names = 'rhoa rhob'
    hj_names = 'ha   hb'
    args = 'etaa0 etab0 etab1'
  []
  [coupled_etab0dot]
    type = ADCoupledSwitchingTimeDerivative
    variable = w
    v = etab0
    Fj_names = 'rhoa rhob'
    hj_names = 'ha   hb'
    args = 'etaa0 etab0 etab1'
  []
  [coupled_etab1dot]
    type = ADCoupledSwitchingTimeDerivative
    variable = w
    v = etab1
    Fj_names = 'rhoa rhob'
    hj_names = 'ha   hb'
    args = 'etaa0 etab0 etab1'
  []
[]

[AuxKernels]
  [BndsCalc]
    type = BndsCalcAux
    variable = bnds
    execute_on = timestep_end
  []
[]

# enable_jit set to false in many materials to make this test start up faster.
# It is recommended to set enable_jit = true or just remove these lines for
# production runs with this model
[Materials]
  [ha]
    type = ADSwitchingFunctionMultiPhaseMaterial
    h_name = ha
    all_etas = 'etaa0 etab0 etab1'
    phase_etas = 'etaa0'
  []
  [hb]
    type = ADSwitchingFunctionMultiPhaseMaterial
    h_name = hb
    all_etas = 'etaa0 etab0 etab1'
    phase_etas = 'etab0 etab1'
  []
  [omegaa]
    type = ADDerivativeParsedMaterial
    args = 'w'
    f_name = omegaa
    material_property_names = 'Vm ka caeq'
    expression = '-0.5*w^2/Vm^2/ka-w/Vm*caeq'
    derivative_order = 2
  []
  [omegab]
    type = ADDerivativeParsedMaterial
    args = 'w'
    f_name = omegab
    material_property_names = 'Vm kb cbeq'
    expression = '-0.5*w^2/Vm^2/kb-w/Vm*cbeq'
    derivative_order = 2
  []
  [rhoa]
    type = ADDerivativeParsedMaterial
    args = 'w'
    f_name = rhoa
    material_property_names = 'Vm ka caeq'
    expression = 'w/Vm^2/ka + caeq/Vm'
    derivative_order = 2
  []
  [rhob]
    type = ADDerivativeParsedMaterial
    args = 'w'
    f_name = rhob
    material_property_names = 'Vm kb cbeq'
    expression = 'w/Vm^2/kb + cbeq/Vm'
    derivative_order = 2
  []
  [const]
    type = ADGenericConstantMaterial
    prop_names =  'kappa_c  kappa   L   D    chi  Vm   ka    caeq kb    cbeq  gab gbb mu'
    prop_values = '0        1       1.0 1.0  1.0  1.0  10.0  0.1  10.0  0.9   4.5 1.5 1.0'
  []
  [Mobility]
    type = ADDerivativeParsedMaterial
    f_name = Dchi
    material_property_names = 'D chi'
    expression = 'D*chi'
    derivative_order = 2
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
  solve_type = NEWTON
  petsc_options_iname = '-pc_type '
  petsc_options_value = 'lu     '
  l_tol = 1.0e-3
  nl_rel_tol = 1.0e-8
  nl_abs_tol = 1e-8
  num_steps = 2
  [TimeStepper]
    type = SolutionTimeAdaptiveDT
    dt = 0.1
  []

[]

[Outputs]
  exodus = true
  file_base = GrandPotentialMultiphase_out
[]
