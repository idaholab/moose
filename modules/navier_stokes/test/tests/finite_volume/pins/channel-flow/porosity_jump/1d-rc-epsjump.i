mu=1.1
rho=1.1
advected_interp_method='upwind'
velocity_interp_method='rc'

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 1
    dx = '1 1'
    ix = '30 30'
    subdomain_id = '1 2'
  []
[]

[GlobalParams]
  rhie_chow_user_object = 'rc'
[]

[UserObjects]
  [rc]
    type = PINSFVRhieChowInterpolator
    u = u
    pressure = pressure
    porosity = porosity
  []
[]

[Variables]
  [u]
    type = PINSFVSuperficialVelocityVariable
    initial_condition = 1
  []
  [pressure]
    type = INSFVPressureVariable
  []
[]

[AuxVariables]
  [porosity]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  []
[]

[ICs]
  inactive = 'porosity_continuous'
  [porosity_1]
    type = ConstantIC
    variable = porosity
    block = 1
    value = 1
  []
  [porosity_2]
    type = ConstantIC
    variable = porosity
    block = 2
    value = 0.5
  []
  [porosity_continuous]
    type = FunctionIC
    variable = porosity
    block = '1 2'
    function = smooth_jump
  []
[]

[Functions]
  [smooth_jump]
    type = ParsedFunction
    value = '1 - 0.5 * 1 / (1 + exp(-30*(x-1)))'
  []
[]

[FVKernels]
  [mass]
    type = PINSFVMassAdvection
    variable = pressure
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
  []

  [u_advection]
    type = PINSFVMomentumAdvection
    variable = u
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    porosity = porosity
    momentum_component = 'x'
  []
  [u_viscosity]
    type = PINSFVMomentumDiffusion
    variable = u
    mu = ${mu}
    porosity = porosity
    momentum_component = 'x'
  []
  [u_pressure]
    type = PINSFVMomentumPressure
    variable = u
    pressure = pressure
    porosity = porosity
    momentum_component = 'x'
  []
[]

[FVBCs]
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = u
    function = '1'
  []
  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'right'
    variable = pressure
    function = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'asm      100                lu           NONZERO'
  line_search = 'none'
[]

[Postprocessors]
  [inlet_p]
    type = SideAverageValue
    variable = 'pressure'
    boundary = 'left'
  []
  [outlet-u]
    type = SideIntegralVariablePostprocessor
    variable = u
    boundary = 'right'
  []
[]

[Outputs]
  exodus = true
  csv = false
[]
