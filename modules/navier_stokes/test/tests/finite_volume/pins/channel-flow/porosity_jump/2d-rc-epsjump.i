mu=1.1
rho=1.1
advected_interp_method='upwind'
velocity_interp_method='rc'

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1'
    dy = '0.5'
    ix = '30 30'
    iy = '20'
    subdomain_id = '1 2'
  []
[]

[GlobalParams]
  two_term_boundary_expansion = true
[]

[Variables]
  [u]
    type = PINSFVSuperficialVelocityVariable
    initial_condition = 1
  []
  [v]
    type = PINSFVSuperficialVelocityVariable
    initial_condition = 1e-6
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
  inactive = 'u_advection_porosity_gradient v_advection_porosity_gradient'
  [mass]
    type = PINSFVMassAdvection
    variable = pressure
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    vel = 'velocity'
    pressure = pressure
    u = u
    v = v
    mu = ${mu}
    rho = ${rho}
    porosity = porosity
  []

  [u_advection]
    type = PINSFVMomentumAdvection
    variable = u
    advected_quantity = 'rhou'
    vel = 'velocity'
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    pressure = pressure
    u = u
    v = v
    mu = ${mu}
    rho = ${rho}
    porosity = porosity
  []
  [u_viscosity]
    type = PINSFVMomentumDiffusion
    variable = u
    mu = ${mu}
    porosity = porosity

    momentum_component = 'x'
    vel = 'velocity'
  []
  [u_pressure]
    type = PINSFVMomentumPressure
    variable = u
    p = pressure
    porosity = porosity
    momentum_component = 'x'
  []
  [u_advection_porosity_gradient]
    type = PINSFVMomentumAdvectionPorosityGradient
    variable = u
    u = u
    v = v
    rho = ${rho}
    porosity = porosity
    momentum_component = 'x'
  []

  [v_advection]
    type = PINSFVMomentumAdvection
    variable = v
    advected_quantity = 'rhov'
    vel = 'velocity'
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    pressure = pressure
    u = u
    v = v
    mu = ${mu}
    rho = ${rho}
    porosity = porosity
  []
  [v_viscosity]
    type = PINSFVMomentumDiffusion
    variable = v
    mu = ${mu}
    porosity = porosity

    momentum_component = 'y'
    vel = 'velocity'
  []
  [v_pressure]
    type = PINSFVMomentumPressure
    variable = v
    p = pressure
    porosity = porosity
    momentum_component = 'y'
  []
  [v_advection_porosity_gradient]
    type = PINSFVMomentumAdvectionPorosityGradient
    variable = v
    u = u
    v = v
    rho = ${rho}
    porosity = porosity
    momentum_component = 'y'
  []
[]

[FVBCs]
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = u
    function = '1'
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = v
    function = 0
  []

  [walls-u]
    type = INSFVNaturalFreeSlipBC
    boundary = 'top bottom'
    variable = u
  []
  [walls-v]
    type = INSFVNaturalFreeSlipBC
    boundary = 'top bottom'
    variable = v
  []

  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'right'
    variable = pressure
    function = 0.4
  []
[]

[Materials]
  [ins_fv]
    type = INSFVMaterial
    u = 'u'
    v = 'v'
    pressure = 'pressure'
    rho = ${rho}
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
