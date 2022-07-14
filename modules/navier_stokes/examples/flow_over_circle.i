mu=7e-4
rho=1
velocity_interp_method = 'rc'
advected_interp_method = 'upwind'
[GlobalParams]
  rhie_chow_user_object = 'rc'
[]
[Mesh]
  [./ccmg]
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
  [side_sets_inlet]
      type = SideSetsFromNormalsGenerator
      input = ccmg
      normals = '1 0 0'
      new_boundary = '100'
  []
  [rename_left]
    type = RenameBoundaryGenerator
    input = side_sets_inlet
    old_boundary = 'left'
    new_boundary = '101'
  []
  [left]
    type = CartesianMeshGenerator
    dim = 2
    dx = '5'
    dy = '1'
    ix = '20'
    iy = '16'
  []
  [move_it]
    type = TransformGenerator
    input = left
    transform = translate
    vector_value = '-5.5 -0.5 0'
  []
  [side_sets_outlet]
      type = SideSetsFromNormalsGenerator
      input = move_it
      normals = '-1 0 0'
      new_boundary = '1000'
  []
  [rename_right]
    type = RenameBoundaryGenerator
    input = side_sets_outlet
    old_boundary = 'right'
    new_boundary = '1001'
  []
  [stitch]
    type = StitchedMeshGenerator
    inputs = 'rename_left rename_right'
    stitch_boundaries_pairs = '101 1001'
  []
  [side_sets_no_slip_top]
    type = SideSetsFromNormalsGenerator
    input = stitch
    normals = '0 1 0'
    new_boundary = 'no_slip_top'
  []
  [side_sets_no_slip_bottom]
    type = SideSetsFromNormalsGenerator
    input = side_sets_no_slip_top
    normals = '0 -1 0'
    new_boundary = 'no_slip_bottom'
  []
  [in_between]
    type = SideSetsBetweenSubdomainsGenerator
    input = side_sets_no_slip_bottom
    primary_block = 2
    paired_block = 1
    new_boundary = 'no_circle'
  []
  [delete]
    type = BlockDeletionGenerator
    input = in_between
    block = '1'
  []
  [rename_inlet]
    type = RenameBoundaryGenerator
    input = delete
    old_boundary = '100'
    new_boundary = 'inlet'
  []
  [rename_outlet]
    type = RenameBoundaryGenerator
    input = rename_inlet
    old_boundary = '1000'
    new_boundary = 'outlet'
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
    boundary = 'no_slip_top no_slip_bottom no_circle'
    function = 0
  []

  [no_slip_y]
    type = INSFVNoSlipWallBC
    variable = vel_y
    boundary = 'no_slip_top no_slip_bottom no_circle'
    function = 0
  []
  [inlet_x]
    type = INSFVInletVelocityBC
    variable = vel_x
    boundary = 'inlet'
    function = inlet_func
  []
  [inlet_y]
    type = INSFVInletVelocityBC
    variable = vel_y
    boundary = 'inlet'
    function = '1e-3 * exp(-t/60) * sin(t)'
  []
  [outlet_p]
    type = INSFVOutletPressureBC
    variable = pressure
    boundary = 'outlet'
    function = 0
  []
[]

[Functions]
  [./inlet_func]
    type = ParsedFunction
    value = '-1'
  [../]
[]

[Materials]
  [./const]
    type = GenericConstantMaterial
    prop_names = 'rho mu'
    prop_values = '${rho}  ${mu}'
  [../]
[]

[Preconditioning]
  [./SMP] #What is PJFNK
    type = SMP
    full = true
    solve_type = 'NEWTON'
  [../]
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
  end_time = 100
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
