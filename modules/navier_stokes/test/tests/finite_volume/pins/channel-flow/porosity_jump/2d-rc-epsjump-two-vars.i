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
    ix = '6 6'
    iy = '4'
    subdomain_id = '1 2'
  []
  [interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = mesh
    primary_block = '1'
    paired_block = '2'
    new_boundary = 'interface'
  []
  [break]
    type = BreakBoundaryOnSubdomainGenerator
    input = interface
    boundaries = 'top bottom'
  []
[]

[UserObjects]
  [rc1]
    type = PINSFVRhieChowInterpolator
    u = u1
    v = v1
    porosity = porosity1
    pressure = pressure1
    block = 1
  []
  [rc2]
    type = PINSFVRhieChowInterpolator
    u = u2
    v = v2
    porosity = porosity2
    pressure = pressure2
    block = 2
  []
[]

[Variables]
  [u1]
    type = PINSFVSuperficialVelocityVariable
    initial_condition = 1
    block = 1
  []
  [v1]
    type = PINSFVSuperficialVelocityVariable
    initial_condition = 1e-6
    block = 1
  []
  [pressure1]
    type = INSFVPressureVariable
    block = 1
  []
  [u2]
    type = PINSFVSuperficialVelocityVariable
    initial_condition = 1
    block = 2
  []
  [v2]
    type = PINSFVSuperficialVelocityVariable
    initial_condition = 1e-6
    block = 2
  []
  [pressure2]
    type = INSFVPressureVariable
    block = 2
  []
[]

[AuxVariables]
  [porosity1]
    type = MooseVariableFVReal
    initial_condition = 1
    block = 1
  []
  [porosity2]
    type = MooseVariableFVReal
    initial_condition = 0.5
    block = 2
  []
[]

[FVKernels]
  [mass1]
    type = PINSFVMassAdvection
    variable = pressure1
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    block = 1
    rhie_chow_user_object = rc1
  []
  [u_advection1]
    type = PINSFVMomentumAdvection
    variable = u1
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    porosity = porosity1
    momentum_component = 'x'
    rhie_chow_user_object = rc1
    block = 1
  []
  [u_viscosity1]
    type = PINSFVMomentumDiffusion
    variable = u1
    mu = ${mu}
    porosity = porosity1
    momentum_component = 'x'
    rhie_chow_user_object = rc1
    block = 1
  []
  [u_pressure1]
    type = PINSFVMomentumPressureFlux
    variable = u1
    pressure = pressure1
    porosity = porosity1
    momentum_component = 'x'
    rhie_chow_user_object = rc1
    boundaries_to_avoid = 'interface'
    block = 1
  []
  [v_advection1]
    type = PINSFVMomentumAdvection
    variable = v1
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    porosity = porosity1
    momentum_component = 'y'
    rhie_chow_user_object = rc1
    block = 1
    boundaries_to_force = 'interface'
  []
  [v_viscosity1]
    type = PINSFVMomentumDiffusion
    variable = v1
    mu = ${mu}
    porosity = porosity1
    momentum_component = 'y'
    rhie_chow_user_object = rc1
    block = 1
    boundaries_to_force = 'interface'
  []
  [v_pressure1]
    type = PINSFVMomentumPressureFlux
    variable = v1
    pressure = pressure1
    porosity = porosity1
    momentum_component = 'y'
    rhie_chow_user_object = rc1
    block = 1
    boundaries_to_force = 'interface'
  []

  [mass2]
    type = PINSFVMassAdvection
    variable = pressure2
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    block = 2
    rhie_chow_user_object = rc2
  []
  [u_advection2]
    type = PINSFVMomentumAdvection
    variable = u2
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    porosity = porosity2
    momentum_component = 'x'
    rhie_chow_user_object = rc2
    block = 2
    boundaries_to_force = 'interface'
  []
  [u_viscosity2]
    type = PINSFVMomentumDiffusion
    variable = u2
    mu = ${mu}
    porosity = porosity2
    momentum_component = 'x'
    rhie_chow_user_object = rc2
    block = 2
    boundaries_to_force = 'interface'
  []
  [u_pressure2]
    type = PINSFVMomentumPressureFlux
    variable = u2
    pressure = pressure2
    porosity = porosity2
    momentum_component = 'x'
    rhie_chow_user_object = rc2
    boundaries_to_force = 'interface'
    block = 2
  []
  [v_advection2]
    type = PINSFVMomentumAdvection
    variable = v2
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    porosity = porosity2
    momentum_component = 'y'
    rhie_chow_user_object = rc2
    block = 2
    boundaries_to_force = 'interface'
  []
  [v_viscosity2]
    type = PINSFVMomentumDiffusion
    variable = v2
    mu = ${mu}
    porosity = porosity2
    momentum_component = 'y'
    rhie_chow_user_object = rc2
    block = 2
    boundaries_to_force = 'interface'
  []
  [v_pressure2]
    type = PINSFVMomentumPressureFlux
    variable = v2
    pressure = pressure2
    porosity = porosity2
    momentum_component = 'y'
    rhie_chow_user_object = rc2
    block = 2
    boundaries_to_force = 'interface'
  []
[]

[FVBCs]
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = u1
    function = '1'
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = v1
    function = 0
  []

  [walls-u1]
    type = INSFVNaturalFreeSlipBC
    boundary = 'top_to_1 bottom_to_1'
    variable = u1
    momentum_component = 'x'
    rhie_chow_user_object = 'rc1'
  []
  [walls-v1]
    type = INSFVNaturalFreeSlipBC
    boundary = 'top_to_1 bottom_to_1'
    variable = v1
    momentum_component = 'y'
    rhie_chow_user_object = 'rc1'
  []
  [walls-u2]
    type = INSFVNaturalFreeSlipBC
    boundary = 'top_to_2 bottom_to_2'
    variable = u2
    momentum_component = 'x'
    rhie_chow_user_object = 'rc2'
  []
  [walls-v2]
    type = INSFVNaturalFreeSlipBC
    boundary = 'top_to_2 bottom_to_2'
    variable = v2
    momentum_component = 'y'
    rhie_chow_user_object = 'rc2'
  []

  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'right'
    variable = pressure2
    function = 0
  []
[]

[FVInterfaceKernels]
  [penalty_mom_u]
    type = PINSFVPenaltyBernoulli
    subdomain1 = '1'
    subdomain2 = '2'
    variable1 = u1
    variable2 = u2
    v1 = v1
    v2 = v2
    pressure1 = pressure1
    pressure2 = pressure2
    porosity1 = porosity1
    porosity2 = porosity2
    rho = ${rho}
    penalty = 1e3
    boundary = 'interface'
  []
  [penalty_mass_u]
    type = PINSFVPenaltyMassContinuity
    subdomain1 = '1'
    subdomain2 = '2'
    variable1 = pressure1
    variable2 = pressure2
    u1 = u1
    u2 = u2
    boundary = 'interface'
    rho = ${rho}
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  line_search = 'none'
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10
[]

[Postprocessors]
  [inlet_p]
    type = SideAverageValue
    variable = 'pressure1'
    boundary = 'left'
  []
  [outlet-u]
    type = SideIntegralVariablePostprocessor
    variable = u2
    boundary = 'right'
  []
[]

[Outputs]
  exodus = true
[]
