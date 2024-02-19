mu = 1.0
rho = 1.0

[Mesh]
  [gen]
    type = CartesianMeshGenerator
    dim = 2
    dx = '0.1 0.8 0.1'
    dy = '0.1 0.8 0.1'
    ix = '5 20 5'
    iy = '5 20 5'
    subdomain_id = '1 1 1
                    1 2 1
                    1 1 1'
  []
  [delete_internal_part]
    type = BlockDeletionGenerator
    input = gen
    block = '2'
    new_boundary = 'wall-internal'
  []
  [lump_bdries_to_wall]
    type = RenameBoundaryGenerator
    input = delete_internal_part
    old_boundary = 'bottom right top left'
    new_boundary = 'wall-external wall-external wall-external wall-external'
  []
  [pump_domain]
    type = ParsedSubdomainMeshGenerator
    input = lump_bdries_to_wall
    combinatorial_geometry = 'x > 0.3 & x < 0.4 & y > 0.5'
    block_id = '3'
  []
  [rename_blocks]
    type = RenameBlockGenerator
    input = pump_domain
    old_block = '1 3'
    new_block = 'pipe pump'
  []
  [side_pump]
    type = ParsedGenerateSideset
    input = rename_blocks
    included_subdomains = 'pump'
    included_neighbors = 'pipe'
    new_sideset_name = 'pump_side'
    normal = '1 0 0'
    combinatorial_geometry = 'x > 0.35'
  []
[]

[GlobalParams]
  velocity_interp_method = 'rc'
  advected_interp_method = 'upwind'
  rhie_chow_user_object = 'rc'
[]

[Problem]
  material_coverage_check = False
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = vel_x
    v = vel_y
    pressure = pressure
    correct_volumetric_force = true
    volumetric_force_functors = 'pump_volume_force'
    volume_force_correction_method = 'pressure-consistent'
  []
[]

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
    initial_condition = 1
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = 1
  []
  [pressure]
    type = INSFVPressureVariable
  []
  [lambda]
    family = SCALAR
    order = FIRST
  []
[]

[AuxVariables]
  [U]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
[]

[AuxKernels]
  [mag]
    type = VectorMagnitudeAux
    variable = U
    x = vel_x
    y = vel_y
  []
[]

[FVKernels]
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    rho = ${rho}
  []
  [mean_zero_pressure]
    type = FVIntegralValueConstraint
    variable = pressure
    lambda = lambda
    phi0 = 0.0
  []

  [u_advection]
    type = INSFVMomentumAdvection
    variable = vel_x
    rho = ${rho}
    momentum_component = 'x'
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_x
    mu = ${mu}
    momentum_component = 'x'
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = vel_x
    momentum_component = 'x'
    pressure = pressure
  []
  [u_pump]
    type = INSFVPump
    variable = vel_x
    momentum_component = 'x'
    pump_volume_force = 'pump_volume_force'
    block = 'pump'
  []

  [v_advection]
    type = INSFVMomentumAdvection
    variable = vel_y
    rho = ${rho}
    momentum_component = 'y'
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
  []
[]

[FVBCs]
  [walls-u]
    type = INSFVNoSlipWallBC
    boundary = 'wall-internal wall-external'
    variable = vel_x
    function = '0'
  []
  [walls-v]
    type = INSFVNoSlipWallBC
    boundary = 'wall-internal wall-external'
    variable = vel_y
    function = '0'
  []
[]

[Functions]
  [pump_head]
    type = PiecewiseLinear
    x = '0.0 10.0'
    y = '1000.0 0.0'
  []
[]

[FunctorMaterials]
  [pump_mat]
    type = NSFVPumpFunctorMaterial
    rho = ${rho}
    speed = 'U'
    pressure_head_function = 'pump_head'
    rotation_speed = 120
    rotation_speed_rated = 100
    area_rated = 0.1
    volume_rated = 0.01
    flow_rate_rated = 1.0
    flow_rate = 'flow_rate'
    block = 'pump'
  []
[]

[Postprocessors]
  [flow_rate]
    type = Receiver
    default = 1.0
  []
  [flow_rate_to_pipe]
    type = VolumetricFlowRate
    advected_quantity = ${rho}
    boundary = 'pump_side'
    vel_x = 'vel_x'
    vel_y = 'vel_y'
  []
  [maximum_speed]
    type = ADElementExtremeFunctorValue
    functor = vel_x
    value_type = max
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = false
  [out]
    type = CSV
    execute_on = FINAL
    show = 'flow_rate_to_pipe maximum_speed'
  []
[]
