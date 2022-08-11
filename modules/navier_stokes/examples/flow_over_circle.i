mu=3.2215e-3
rho=1
velocity_interp_method = 'rc'
advected_interp_method = 'upwind'

[GlobalParams]
  rhie_chow_user_object = 'rc'
[]
[Mesh]
  [ccmg]
    type = ConcentricCircleMeshGenerator
    num_sectors = 6
    radii = '0.2546 0.3368'
    rings = '4 3 4'
    has_outer_square = on
    pitch = 1
    #portion = left_half
    preserve_volumes = off
    smoothing_max_it = 3
  []
  [in_between]
    type = SideSetsBetweenSubdomainsGenerator
    input = ccmg
    primary_block = 2
    paired_block = 1
    new_boundary = 'no_circle'
  []
  [delete]
    type = BlockDeletionGenerator
    input = in_between
    block = '1'
  []
  [final_ccmg]
    type = RenameBlockGenerator
    input = delete
    old_block = '2 3'
    new_block = '0 0'
  []
  [left]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -4.5
    xmax = -0.5
    ymin = -0.5
    ymax = 0.5
    nx = '80'
    ny = '16'
  []
  [right]
    type = GeneratedMeshGenerator
    xmin = 0.5
    xmax = 4.5
    ymin = -0.5
    ymax = 0.5
    nx = '80'
    ny = '16'
    dim = 2
  []
  [stitch_left]
    type = StitchedMeshGenerator
    inputs = 'final_ccmg left'
    stitch_boundaries_pairs = 'left right'
  []
  [combined_middle]
    type = StitchedMeshGenerator
    inputs = 'stitch_left right'
    stitch_boundaries_pairs = 'right left'
  []

  [middle_top_sideset]
    input = combined_middle
    type = ParsedGenerateSideset
    combinatorial_geometry = 'y > 0.49999999'
    normal = '0 1 0'
    new_sideset_name = 'middle_top'
  []
  [middle_bottom_sideset]
    input = middle_top_sideset
    type = ParsedGenerateSideset
    combinatorial_geometry = 'y < -0.4999999'
    normal = '0 -1 0'
    new_sideset_name = 'bottom_boundary'
  []

  [top_left_block]
    type = GeneratedMeshGenerator
    xmin = -4.5
    xmax = -0.5
    ymin = 0.5
    ymax = ${fparse 0.5 + 2. / 16.}
    nx = 80
    ny = 2
    dim = 2
  []
  [top_middle_block]
    type = GeneratedMeshGenerator
    xmin = -0.5
    xmax = 0.5
    ymin = 0.5
    ymax = ${fparse 0.5 + 2. / 16.}
    nx = 16
    ny = 2
    dim = 2
  []
  [top_right_block]
    type = GeneratedMeshGenerator
    xmin = 0.5
    xmax = 4.5
    ymin = 0.5
    ymax = ${fparse 0.5 + 2. / 16.}
    nx = 80
    ny = 2
    dim = 2
  []
  [stitch_top_left]
    type = StitchedMeshGenerator
    inputs = 'top_middle_block top_left_block'
    stitch_boundaries_pairs = 'left right'
  []
  [combined_top]
    type = StitchedMeshGenerator
    inputs = 'stitch_top_left top_right_block'
    stitch_boundaries_pairs = 'right left'
  []
  [top_bottom_sideset]
    input = combined_top
    type = ParsedGenerateSideset
    combinatorial_geometry = 'y < 0.5000001'
    normal = '0 -1 0'
    new_sideset_name = 'top_bottom'
  []
  [combined_middle_top]
    type = StitchedMeshGenerator
    inputs = 'top_bottom_sideset middle_bottom_sideset'
    stitch_boundaries_pairs = 'top_bottom middle_top'
  []
  [create_fused_top_sideset]
    input = combined_middle_top
    type = ParsedGenerateSideset
    combinatorial_geometry = 'y > ${fparse 0.999999 * 0.5 + 2. / 16.}'
    normal = '0 1 0'
    new_sideset_name = 'top_boundary'
  []
  [create_fused_left_sideset]
    input = create_fused_top_sideset
    type = ParsedGenerateSideset
    combinatorial_geometry = 'x < -4.499999'
    normal = '-1 0 0'
    new_sideset_name = 'left_boundary'
  []
  [create_fused_right_sideset]
    input = create_fused_left_sideset
    type = ParsedGenerateSideset
    combinatorial_geometry = 'x > 4.4999999'
    normal = '1 0 0'
    new_sideset_name = 'right_boundary'
  []
  uniform_refine = 2
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = vel_x
    v = vel_y
    pressure = pressure
  []
[]

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
    two_term_boundary_expansion = true
  []
  [vel_y]
    type = INSFVVelocityVariable
    two_term_boundary_expansion = true
  []
  [pressure]
    type = INSFVPressureVariable
    two_term_boundary_expansion = true
  []
[]

[FVKernels]
  # mass
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
  []
  [u_time]
    type = INSFVMomentumTimeDerivative
    variable = vel_x
    rho = ${rho}
    momentum_component = 'x'
  []
  [u_advection]
    type = INSFVMomentumAdvection
    variable = vel_x
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
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
  [v_time]
    type = INSFVMomentumTimeDerivative
    variable = vel_y
    rho = ${rho}
    momentum_component = 'y'
  []
  [v_advection]
    type = INSFVMomentumAdvection
    variable = vel_y
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
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
  [no_slip_x]
    type = INSFVNoSlipWallBC
    variable = vel_x
    boundary = 'top_boundary bottom_boundary no_circle'
    function = 0
  []
  [no_slip_y]
    type = INSFVNoSlipWallBC
    variable = vel_y
    boundary = 'top_boundary bottom_boundary no_circle'
    function = 0
  []
  [inlet_x]
    type = INSFVInletVelocityBC
    variable = vel_x
    boundary = 'left_boundary'
    function = inlet_func
  []
  [inlet_y]
    type = INSFVInletVelocityBC
    variable = vel_y
    boundary = 'left_boundary'
    function = 0
  []
  [outlet_p]
    type = INSFVOutletPressureBC
    variable = pressure
    boundary = 'right_boundary'
    function = 0
  []
[]

[Functions]
  [inlet_func]
    type = ParsedFunction
    value = '-1'
  []
[]

[Materials]
  [const]
    type = GenericConstantMaterial
    prop_names = 'rho mu'
    prop_values = '${rho}  ${mu}'
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
    solve_type = 'NEWTON'
  []
[]

[Executioner]
  type = Transient
  dtmin = 1e-5
  petsc_options = '-snes_converged_reason -ksp_converged_reason'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  line_search = 'none'
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-12
  nl_max_its = 10
  end_time = 15
  dtmax = 1
  scheme = 'bdf2'
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1e-5
    optimal_iterations = 6
    growth_factor = 1.5
  []
[]

[Outputs]
  exodus = true
  csv = true
  checkpoint = true
[]

[Postprocessors]
  [Re]
    type = ParsedPostprocessor
    function = 'rho * U * D / mu'
    constant_names = 'rho U D mu'
    constant_expressions = '${rho} 1 ${fparse 2 * .2546} ${mu}'
    pp_names = ''
  []
  [Strouhal_Number]
    type = ParsedPostprocessor
    function = '0.198*(1-(19.7/(rho*U*D/mu)))'
    constant_names = 'rho U D mu'
    constant_expressions = '${rho} 1 ${fparse 2 * .2546} ${mu}'
    pp_names = ''
  []
  [Strouhal_Number_PVS]
    type = ParsedPostprocessor
    function = '.285 + (-1.3897/sqrt(rho*U*D/mu)) + 1.8061/(rho*U*D/mu)'
    constant_names = 'rho U D mu'
    constant_expressions = '${rho} 1 ${fparse 2 * .2546} ${mu}'
    pp_names = ''
  []
  [Frequency]
    type = ParsedPostprocessor
    function = '((0.198*U)/9)*(1-(19.7/(rho*U*D/mu)))'
    constant_names = 'rho U D mu'
    constant_expressions = '${rho} 1 ${fparse 2 * .2546} ${mu}'
    pp_names = ''
  []
  [element_44146_x]
    type = ElementalVariableValue
    variable = 'vel_x'
    elementid = 44146
  []
  [element_44146_y]
    type = ElementalVariableValue
    variable = 'vel_y'
    elementid = 44146
  []
[]
