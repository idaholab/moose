# Water properties
mu = 1.0E-3
rho = 1000.0
k = 0.598
cp = 4186

# Solid properties
cp_s = 830
rho_s = 1680
k_s = 3.5

# Other parameters
p_outlet = 0
u_inlet = -1e-4
h_conv = 50

[Mesh]
  [generated_mesh]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    xmin = 0
    ymin = 0
    ymax = 0.1
    xmax = 0.1
  []
  [subdomain1]
    input = generated_mesh
    type = SubdomainBoundingBoxGenerator
    block_name = subdomain1
    bottom_left = '0.04 0.04 0'
    block_id = 1
    top_right = '0.06 0.06 0'
  []
  [interface]
    input = subdomain1
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = 0
    paired_block = 1
    new_boundary = interface
  []
[]

[GlobalParams]
  rhie_chow_user_object = 'rc'
  advected_interp_method = 'upwind'
  velocity_interp_method = 'rc'
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = vel_x
    v = vel_y
    pressure = pressure
    block = 0
  []
[]

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
    initial_condition = 1e-4
    block = 0
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = 1e-4
    block = 0
  []
  [pressure]
    type = INSFVPressureVariable
    block = 0
  []
  [T]
    type = INSFVEnergyVariable
    initial_condition = 283.15
    scaling = 1e-5
    block = 0
  []
  [Ts]
    type = INSFVEnergyVariable
    initial_condition = 333.15
    scaling = 1e-5
    block = 1
  []
[]

[FVKernels]
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    rho = ${rho}
    block = 0
  []
  [u_time]
    type = INSFVMomentumTimeDerivative
    variable = vel_x
    rho = ${rho}
    momentum_component = 'x'
    block = 0
  []

  [u_advection]
    type = INSFVMomentumAdvection
    variable = vel_x
    rho = ${rho}
    momentum_component = 'x'
    block = 0
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_x
    mu = ${mu}
    momentum_component = 'x'
    block = 0
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = vel_x
    momentum_component = 'x'
    pressure = pressure
    block = 0
  []
  [v_time]
    type = INSFVMomentumTimeDerivative
    variable = vel_y
    rho = ${rho}
    momentum_component = 'y'
    block = 0
  []
  [v_advection]
    type = INSFVMomentumAdvection
    variable = vel_y
    rho = ${rho}
    momentum_component = 'y'
    block = 0
  []
  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_y
    mu = ${mu}
    momentum_component = 'y'
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = vel_y
    momentum_component = 'y'
    pressure = pressure
    block = 0
  []
  [energy_time]
    type = INSFVEnergyTimeDerivative
    variable = T
    cp = ${cp}
    rho = ${rho}
    block = 0
  []
  [temp_conduction]
    type = FVDiffusion
    coeff = 'k'
    variable = T
    block = 0
  []
  [temp_advection]
    type = INSFVEnergyAdvection
    variable = T
    block = 0
  []
  [solid_energy_time]
    type = INSFVEnergyTimeDerivative
    variable = Ts
    cp = ${cp_s}
    rho = ${rho_s}
    block = 1
  []
  [solid_temp_conduction]
    type = FVDiffusion
    coeff = 'k_s'
    variable = Ts
    block = 1
  []
[]

[FVInterfaceKernels]
  [convection]
    type = FVConvectionCorrelationInterface
    variable1 = T
    variable2 = Ts
    subdomain1 = 0
    subdomain2 = 1
    boundary = interface
    h = ${h_conv}
    T_solid = Ts
    T_fluid = T
    wall_cell_is_bulk = true
  []
[]

[FVBCs]
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'top'
    variable = vel_x
    function = 0
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'top'
    variable = vel_y
    function = ${u_inlet}
  []
  [inlet_T]
    type = FVDirichletBC
    variable = T
    boundary = 'top'
    value = 283.15
  []
  [no-slip-u]
    type = INSFVNoSlipWallBC
    boundary = 'left right interface'
    variable = vel_x
    function = 0
  []
  [no-slip-v]
    type = INSFVNoSlipWallBC
    boundary = 'left right interface'
    variable = vel_y
    function = 0
  []

  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'bottom'
    variable = pressure
    function = '${p_outlet}'
  []
[]

[Materials]
  [functor_constants]
    type = ADGenericFunctorMaterial
    prop_names = 'cp k'
    prop_values = '${cp} ${k}'
    block = 0
  []
  [ins_fv]
    type = INSFVEnthalpyMaterial
    temperature = 'T'
    rho = ${rho}
    block = 0
  []
  [solid_functor_constants]
    type = ADGenericFunctorMaterial
    prop_names = 'cp_s k_s'
    prop_values = '${cp_s} ${k_s}'
    block = 1
  []
  [solid_ins_fv]
    type = INSFVEnthalpyMaterial
    temperature = 'Ts'
    rho = ${rho_s}
    block = 1
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'asm      100                lu           NONZERO'
  line_search = 'none'
  nl_rel_tol = 1e-8
  dt = 10
  end_time = 10
[]

[Outputs]
  exodus = true
[]
