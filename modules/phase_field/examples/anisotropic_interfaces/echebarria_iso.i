#This example implements an isotropic, isothermal version of the
#binary alloy solidification model of Echebarria et al.,
#Physical Review E, 70, 061604 (2004). The governing equations are (132)-(133)
#Temperature gradient, pulling velocity, and interfacial energy anisotropy
#are not included.
#The sinusoidal perturbation at the interface decays appproximately
#exponentially with approximate decay constant 1.55e-4, in agreement
#with linear stability analysis
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 120
  ny = 300
  xmin = 0
  xmax = 96
  ymin = 0
  ymax = 240
[]

[GlobalParams]
  enable_jit = true
  derivative_order = 2
[]

[Variables]
  [phi]
  []
  [U]
  []
[]

[AuxVariables]
  [c]
  []
[]

[ICs]
  [phi_IC]
    type = FunctionIC
    variable = phi
    function = ic_func_phi
  []
  [U_IC]
    type = FunctionIC
    variable = U
    function = ic_func_U
  []
[]

[Functions]
  [ic_func_phi]
    type = ParsedFunction
    symbol_names = 'midpoint lambda A'
    symbol_values = '40       96     8'
    expression = 'tanh(-(y - (midpoint + A * cos(2 * pi * x / lambda))) / sqrt(2))'
  []
  [ic_func_U]
    type = ParsedFunction
    expression = '0'
  []
[]

[Kernels]
  # Order parameter phi
  [AC_dphi_dt]
    type = SusceptibilityTimeDerivative
    variable = phi
    f_name = dphi_dt_pre
  []
  [AC_grad]
    type = MatDiffusion
    variable = phi
    diffusivity = as_sq
  []
  [AC_floc]
    type = AllenCahn
    variable = phi
    f_name = f_loc
    mob_name = L
    coupled_variables = 'U'
  []
  #dimensionless supersaturation U
  [diff_dU_dt]
    type = SusceptibilityTimeDerivative
    variable = U
    f_name = dU_dt_pre
    coupled_variables = 'phi'
  []
  [diff_grad]
    type = MatDiffusion
    variable = U
    diffusivity = D_interp
    args = 'phi'
  []
  [diff_antitrapping]
    type = AntitrappingCurrent
    variable = U
    v = phi
    f_name = antitrap_pre
  []
  [diff_dphidt]
    type = CoupledSusceptibilityTimeDerivative
    variable = U
    v = phi
    f_name = coupled_pre
  []
[]

[AuxKernels]
  [c_aux]
    type = ParsedAux
    variable = c
    constant_names = 'c_l   k'
    constant_expressions = '0.33  0.1712'
    coupled_variables = 'phi U'
    expression = '(1 + (1-k) * U) / 2 * c_l * (1+k - (1-k)*phi)'
  []
[]

[Materials]
  [dphi_dt_pre_material]
    type = DerivativeParsedMaterial
    property_name = dphi_dt_pre
    material_property_names = 'as_sq(phi) k'
    expression = '(1-(1-k)*0) * as_sq'
  []
  [as_sq_material]
    type = DerivativeParsedMaterial
    property_name = as_sq
    expression = '1'
  []
  [f_loc_material]
    type = DerivativeParsedMaterial
    property_name = f_loc
    coupled_variables = 'phi U'
    constant_names = 'a1'
    constant_expressions = '5*sqrt(2)/8'
    material_property_names = 'epsilon'
    expression = '-phi^2/2 + phi^4/4 + a1 * epsilon * (phi - 2*phi^3/3 + phi^5/5) * U'
  []
  [dU_dt_pre_material]
    type = DerivativeParsedMaterial
    property_name = dU_dt_pre
    coupled_variables = 'phi'
    material_property_names = 'k'
    expression = '(1+k)/2 - (1-k)/2 * phi'
  []
  [D_interp_material]
    type = DerivativeParsedMaterial
    property_name = D_interp
    coupled_variables = 'phi'
    material_property_names = 'epsilon'
    constant_names = '      a1           a2'
    constant_expressions = '5*sqrt(2)/8  0.6267'
    expression = 'a1 * a2 * epsilon * (1-phi)/2'
    # expression = 'a1 * a2 * epsilon'
    output_properties = 'D_interp'
    outputs = 'exodus'
  []
  [antitrap_pre_material]
    type = DerivativeParsedMaterial
    property_name = antitrap_pre
    coupled_variables = 'U'
    material_property_names = 'k'
    expression = '1/(2*sqrt(2)) * (1 + (1-k) * U)'
  []
  [coupled_pre_material]
    type = DerivativeParsedMaterial
    property_name = coupled_pre
    coupled_variables = 'U'
    material_property_names = 'k'
    expression = '- (1 + (1-k) * U) / 2'
  []
  [const]
    type = GenericConstantMaterial
    prop_names = ' L   k      epsilon'
    prop_values = '1.0 0.1712 30     '
  []
[]

[Postprocessors]
  [int_position]
    type = FindValueOnLine
    start_point = '0 0 0'
    end_point = '0 100 0'
    v = phi
    target = 0
    tol = 1e-8
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
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_pc_type -pc_asm_overlap'
  petsc_options_value = 'asm      31                  lu           1'
  l_tol = 1.0e-3
  nl_rel_tol = 1.0e-8
  nl_abs_tol = 1e-9
  end_time = 1e9
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1
    optimal_iterations = 8
    iteration_window = 2
  []
[]

[Outputs]
  exodus = true
  csv = true
[]
