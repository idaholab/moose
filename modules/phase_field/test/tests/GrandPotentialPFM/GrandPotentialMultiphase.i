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
  [./w]
  [../]
  [./etaa0]
  [../]
  [./etab0]
  [../]
  [./etab1]
  [../]
[]

[AuxVariables]
  [./bnds]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[ICs]

  [./IC_etaa0]
    type = FunctionIC
    variable = etaa0
    function = ic_func_etaa0
  [../]
  [./IC_etab0]
    type = FunctionIC
    variable = etab0
    function = ic_func_etab0
  [../]
  [./IC_etab1]
    type = FunctionIC
    variable = etab1
    function = ic_func_etab1
  [../]
  [./IC_w]
    type = ConstantIC
    value = -0.05
    variable = w
  [../]
[]

[Functions]
  [./ic_func_etaa0]
    type = ParsedFunction
    expression = 'r:=sqrt(x^2+y^2);0.5*(1.0-tanh((r-10.0)/sqrt(2.0)))'
  [../]
  [./ic_func_etab0]
    type = ParsedFunction
    expression = 'r:=sqrt(x^2+y^2);0.5*(1.0+tanh((r-10)/sqrt(2.0)))*0.5*(1.0+tanh((y)/sqrt(2.0)))'
  [../]
  [./ic_func_etab1]
    type = ParsedFunction
    expression = 'r:=sqrt(x^2+y^2);0.5*(1.0+tanh((r-10)/sqrt(2.0)))*0.5*(1.0-tanh((y)/sqrt(2.0)))'
  [../]
[]


[BCs]
[]

[Kernels]
# Order parameter eta_alpha0
  [./ACa0_bulk]
    type = ACGrGrMulti
    variable = etaa0
    v =           'etab0 etab1'
    gamma_names = 'gab   gab'
  [../]
  [./ACa0_sw]
    type = ACSwitching
    variable = etaa0
    Fj_names  = 'omegaa omegab'
    hj_names  = 'ha     hb'
    coupled_variables = 'etab0 etab1 w'
  [../]
  [./ACa0_int]
    type = ACInterface
    variable = etaa0
    kappa_name = kappa
  [../]
  [./ea0_dot]
    type = TimeDerivative
    variable = etaa0
  [../]
# Order parameter eta_beta0
  [./ACb0_bulk]
    type = ACGrGrMulti
    variable = etab0
    v =           'etaa0 etab1'
    gamma_names = 'gab   gbb'
  [../]
  [./ACb0_sw]
    type = ACSwitching
    variable = etab0
    Fj_names  = 'omegaa omegab'
    hj_names  = 'ha     hb'
    coupled_variables = 'etaa0 etab1 w'
  [../]
  [./ACb0_int]
    type = ACInterface
    variable = etab0
    kappa_name = kappa
  [../]
  [./eb0_dot]
    type = TimeDerivative
    variable = etab0
  [../]
# Order parameter eta_beta1
  [./ACb1_bulk]
    type = ACGrGrMulti
    variable = etab1
    v =           'etaa0 etab0'
    gamma_names = 'gab   gbb'
  [../]
  [./ACb1_sw]
    type = ACSwitching
    variable = etab1
    Fj_names  = 'omegaa omegab'
    hj_names  = 'ha     hb'
    coupled_variables = 'etaa0 etab0 w'
  [../]
  [./ACb1_int]
    type = ACInterface
    variable = etab1
    kappa_name = kappa
  [../]
  [./eb1_dot]
    type = TimeDerivative
    variable = etab1
  [../]
#Chemical potential
  [./w_dot]
    type = SusceptibilityTimeDerivative
    variable = w
    f_name = chi
    coupled_variables = '' # in this case chi (the susceptibility) is simply a constant
  [../]
  [./Diffusion]
    type = MatDiffusion
    variable = w
    diffusivity = Dchi
    args = ''
  [../]
  [./coupled_etaa0dot]
    type = CoupledSwitchingTimeDerivative
    variable = w
    v = etaa0
    Fj_names = 'rhoa rhob'
    hj_names = 'ha   hb'
    coupled_variables = 'etaa0 etab0 etab1'
  [../]
  [./coupled_etab0dot]
    type = CoupledSwitchingTimeDerivative
    variable = w
    v = etab0
    Fj_names = 'rhoa rhob'
    hj_names = 'ha   hb'
    coupled_variables = 'etaa0 etab0 etab1'
  [../]
  [./coupled_etab1dot]
    type = CoupledSwitchingTimeDerivative
    variable = w
    v = etab1
    Fj_names = 'rhoa rhob'
    hj_names = 'ha   hb'
    coupled_variables = 'etaa0 etab0 etab1'
  [../]
[]

[AuxKernels]
  [./BndsCalc]
    type = BndsCalcAux
    variable = bnds
    execute_on = timestep_end
  [../]
[]

# enable_jit set to false in many materials to make this test start up faster.
# It is recommended to set enable_jit = true or just remove these lines for
# production runs with this model
[Materials]
  [./ha]
    type = SwitchingFunctionMultiPhaseMaterial
    h_name = ha
    all_etas = 'etaa0 etab0 etab1'
    phase_etas = 'etaa0'
  [../]
  [./hb]
    type = SwitchingFunctionMultiPhaseMaterial
    h_name = hb
    all_etas = 'etaa0 etab0 etab1'
    phase_etas = 'etab0 etab1'
  [../]
  [./omegaa]
    type = DerivativeParsedMaterial
    coupled_variables = 'w'
    property_name = omegaa
    material_property_names = 'Vm ka caeq'
    expression = '-0.5*w^2/Vm^2/ka-w/Vm*caeq'
    derivative_order = 2
    enable_jit = false
  [../]
  [./omegab]
    type = DerivativeParsedMaterial
    coupled_variables = 'w'
    property_name = omegab
    material_property_names = 'Vm kb cbeq'
    expression = '-0.5*w^2/Vm^2/kb-w/Vm*cbeq'
    derivative_order = 2
    enable_jit = false
  [../]
  [./rhoa]
    type = DerivativeParsedMaterial
    coupled_variables = 'w'
    property_name = rhoa
    material_property_names = 'Vm ka caeq'
    expression = 'w/Vm^2/ka + caeq/Vm'
    derivative_order = 2
    enable_jit = false
  [../]
  [./rhob]
    type = DerivativeParsedMaterial
    coupled_variables = 'w'
    property_name = rhob
    material_property_names = 'Vm kb cbeq'
    expression = 'w/Vm^2/kb + cbeq/Vm'
    derivative_order = 2
    enable_jit = false
  [../]
  [./const]
    type = GenericConstantMaterial
    prop_names =  'kappa_c  kappa   L   D    chi  Vm   ka    caeq kb    cbeq  gab gbb mu'
    prop_values = '0        1       1.0 1.0  1.0  1.0  10.0  0.1  10.0  0.9   4.5 1.5 1.0'
  [../]
  [./Mobility]
    type = DerivativeParsedMaterial
    property_name = Dchi
    material_property_names = 'D chi'
    expression = 'D*chi'
    derivative_order = 2
    enable_jit = false
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
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_pc_type -pc_asm_overlap'
  petsc_options_value = 'asm      31                  lu           1'
  l_tol = 1.0e-3
  nl_rel_tol = 1.0e-8
  nl_abs_tol = 1e-8
  num_steps = 2
  [./TimeStepper]
    type = SolutionTimeAdaptiveDT
    dt = 0.1
  [../]

[]

[Outputs]
  exodus = true
[]
