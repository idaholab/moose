L = 30
bulk_u = 0.01
p_ref = 101325.0
T_in = 860
q_source = 50000

A_cp = 976.78
B_cp = 1.0634
rho = 2000

advected_interp_method = 'upwind'

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 0
    xmax = ${L}
    nx = 10
  []
[]

[GlobalParams]
  rhie_chow_user_object = 'rc'
  advected_interp_method = ${advected_interp_method}
  velocity_interp_method = 'rc'
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = vel_x
    pressure = pressure
  []
[]

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
    initial_condition = ${bulk_u}
    two_term_boundary_expansion = false
  []
  [pressure]
    type = INSFVPressureVariable
    initial_condition = ${p_ref}
    two_term_boundary_expansion = false
  []
  [T]
    type = INSFVEnergyVariable
    two_term_boundary_expansion = false
    initial_condition = ${T_in}
  []
[]

[FVKernels]
  [mass]
    type = WCNSFVMassAdvection
    variable = pressure
    rho = 'rho'
  []

  [u_advection]
    type = INSFVMomentumAdvection
    variable = vel_x
    rho = 'rho'
    momentum_component = 'x'
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_x
    mu = 'mu'
    momentum_component = 'x'
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = vel_x
    momentum_component = 'x'
    pressure = pressure
  []

  [temp_conduction]
    type = FVDiffusion
    coeff = 'k'
    variable = T
  []
  [temp_advection]
    type = INSFVEnergyAdvection
    variable = T
  []
  [source]
    type = FVBodyForce
    variable = T
    function = source_func
  []
[]

[FVBCs]
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = vel_x
    functor = ${bulk_u}
  []
  [inlet_T]
    type = FVDirichletBC
    variable = T
    boundary = 'left'
    value = ${T_in}
  []

  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'right'
    variable = pressure
    function = ${p_ref}
  []
[]

[Functions]
  [source_func]
    type = ParsedFunction
    expression = '${q_source}'
  []
[]

[FunctorMaterials]
  [converter_to_regular_T]
    type = FunctorADConverter
    ad_props_in = 'T'
    reg_props_out = 'T_nAD'
  []
  [ins_fv]
    type = INSFVEnthalpyFunctorMaterial
    temperature = 'T'
    rho = 'rho'
    cp = 'cp'

    assumed_constant_cp = false
    h_in = 'h'
    # Alternative to providing 'h': set the fluid property and the pressure parameter
    # fp = 'fp'
    # pressure = 'pressure'
  []

  [rho]
    type = ADParsedFunctorMaterial
    property_name = 'rho'
    expression = '${rho}'
  []
  [mu]
    type = ADParsedFunctorMaterial
    property_name = 'mu'
    expression = '4.5e-3'
  []
  [k]
    type = ADParsedFunctorMaterial
    property_name = 'k'
    expression = '0.7'
  []
  [h]
    type = ADParsedFunctorMaterial
    property_name = 'h'
    functor_names = 'T ${A_cp} ${B_cp}'
    functor_symbols = 'T A_cp B_cp'
    expression = 'A_cp * T + B_cp * T * T / 2'
  []
  [cp]
    type = ADParsedFunctorMaterial
    property_name = 'cp'
    functor_names = 'T ${A_cp} ${B_cp}'
    functor_symbols = 'T A_cp B_cp'
    expression = 'A_cp+B_cp*T'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'

  nl_abs_tol = 1e-9
  nl_max_its = 50
  line_search = 'none'

  automatic_scaling = true
  off_diagonals_in_auto_scaling = true
[]

[Postprocessors]
  [H_in]
    type = VolumetricFlowRate
    vel_x = 'vel_x'
    advected_quantity = 'rho_h'
    boundary = 'left'
  []
  [H_out]
    type = VolumetricFlowRate
    vel_x = 'vel_x'
    advected_quantity = 'rho_h'
    boundary = 'right'
  []
  [Q]
    type = FunctionElementIntegral
    function = 'source_func'
    execute_on = 'initial'
  []
  [balance_in_percent]
    type = ParsedPostprocessor
    expression = '(H_out + H_in - Q) / H_in * 100'
    pp_names = 'H_in H_out Q'
  []

  [T_out]
    type = SideAverageValue
    variable = T
    boundary = 'right'
  []
  [T_analytical_outlet]
    type = Receiver
    default = ${fparse (-A_cp+sqrt(A_cp^2-2*B_cp*(-q_source/rho/bulk_u*L-A_cp*T_in-B_cp/2*T_in*T_in)))/B_cp}
  []
  [error_T]
    type = ParsedPostprocessor
    expression = 'T_out - T_analytical_outlet'
    pp_names = 'T_out T_analytical_outlet'
  []
[]

[Outputs]
  csv = true
[]
