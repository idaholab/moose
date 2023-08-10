D = 0.0254
rho = 1.0
Re = 7.43e4
U = 44.38
mu = '${fparse rho * U * D / Re}'
mixing_length_entry = 1.0e-4
mixing_length_devel = 3.4e-3
mixing_length_asymp = 9.0e-3

advected_interp_method = 'upwind'
velocity_interp_method = 'rc'

[GlobalParams]
  rhie_chow_user_object = 'rc'
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = vel_x
    v = vel_y
    pressure = pressure
  []
[]

[Mesh]
  coord_type = 'RZ'
  rz_coord_axis = 'X'
  uniform_refine = 0
  [gen]
    type = CartesianMeshGenerator
    dim = 2
    dx = '${fparse D}   ${fparse 3*D} ${fparse 7*D} ${fparse 20*D}'
    dy = '${fparse D/2} ${fparse (20*D - D)/2}'
    ix = '20 20 20 40'
    iy = '5 80'
    subdomain_id = '1 1 3 4
                    2 1 3 4'
  []
  [create_1_2_bdry]
    type = SideSetsBetweenSubdomainsGenerator
    input = 'gen'
    primary_block = 1
    paired_block = 2
    new_boundary = 'between_1_2'
  []
  [delte_top]
    type = BlockDeletionGenerator
    input = 'create_1_2_bdry'
    block = 2
  []
  [rename_block]
    type = RenameBlockGenerator
    input = 'delte_top'
    old_block = '1 3 4'
    new_block = 'entry development asymptotic'
  []
  [rename_boundaries]
    type = RenameBoundaryGenerator
    input = 'rename_block'
    old_boundary = 'left  right  top      between_1_2 bottom'
    new_boundary = 'inlet outlet wall_top wall        symmetry'
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
[]

[FVKernels]
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
  [u_viscosity_rans]
    type = INSFVMixingLengthReynoldsStress
    variable = vel_x
    rho = ${rho}
    mixing_length = 'mixing_length'
    momentum_component = 'x'
    u = vel_x
    v = vel_y
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
  [v_viscosity_rans]
    type = INSFVMixingLengthReynoldsStress
    variable = vel_y
    rho = ${rho}
    mixing_length = 'mixing_length'
    momentum_component = 'y'
    u = vel_x
    v = vel_y
  []
[]

[FVBCs]
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'inlet'
    variable = vel_x
    function = '${U}'
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'inlet'
    variable = vel_y
    function = '0'
  []
  [sym_u]
    type = INSFVSymmetryVelocityBC
    boundary = 'symmetry'
    variable = vel_x
    u = vel_x
    v = vel_y
    mu = ${mu}
    momentum_component = 'x'
  []
  [sym_v]
    type = INSFVSymmetryVelocityBC
    boundary = 'symmetry'
    variable = vel_y
    u = vel_x
    v = vel_y
    mu = ${mu}
    momentum_component = y
  []
  [walls-u]
    type = INSFVWallFunctionBC
    variable = vel_x
    boundary = 'wall'
    u = vel_x
    v = vel_y
    mu = ${mu}
    rho = ${rho}
    momentum_component = 'x'
  []
  [walls-v]
    type = INSFVWallFunctionBC
    variable = vel_y
    boundary = 'wall'
    u = vel_x
    v = vel_y
    mu = ${mu}
    rho = ${rho}
    momentum_component = 'y'
  []
  [wall-top-u]
    type = INSFVNaturalFreeSlipBC
    variable = vel_x
    boundary = 'wall_top'
    momentum_component = 'x'
  []
  [wall-top-v]
    type = INSFVNaturalFreeSlipBC
    variable = vel_y
    boundary = 'wall_top'
    momentum_component = 'y'
  []
  # [walls-u]
  #   type = INSFVNoSlipWallBC
  #   boundary = 'wall'
  #   variable = vel_x
  #   function = 0
  # []
  # [walls-v]
  #   type = INSFVNoSlipWallBC
  #   boundary = 'wall'
  #   variable = vel_y
  #   function = 0
  # []
  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'outlet'
    variable = pressure
    function = '0'
  []
[]

[Materials]
  [mixing_length]
    type = PiecewiseByBlockFunctorMaterial
    prop_name = 'mixing_length'
    subdomain_to_prop_value = '1 ${mixing_length_entry}
                               3 ${mixing_length_devel}
                               4 ${mixing_length_asymp}'
  []
[]

[Executioner]
  type = Transient
  steady_state_detection = false
  solve_type = 'NEWTON'
  nl_abs_tol = 1e-8
  nl_max_its = 10
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1e-3
    timestep_limiting_postprocessor = limit_dt
  []
  end_time = 1e6
[]

[Preconditioning]
  active = SMP
  [FSP]
    type = FSP
    # It is the starting point of splitting
    topsplit = 'up' # 'up' should match the following block name
    [up]
      splitting = 'u p' # 'u' and 'p' are the names of subsolvers
      splitting_type = schur
      # Splitting type is set as schur, because the pressure part of Stokes-like systems
      # is not diagonally dominant. CAN NOT use additive, multiplicative and etc.
      #
      # Original system:
      #
      # | Auu Aup | | u | = | f_u |
      # | Apu 0   | | p |   | f_p |
      #
      # is factorized into
      #
      # |I             0 | | Auu  0|  | I  Auu^{-1}*Aup | | u | = | f_u |
      # |Apu*Auu^{-1}  I | | 0   -S|  | 0  I            | | p |   | f_p |
      #
      # where
      #
      # S = Apu*Auu^{-1}*Aup
      #
      # The preconditioning is accomplished via the following steps
      #
      # (1) p* = f_p - Apu*Auu^{-1}f_u,
      # (2) p = (-S)^{-1} p*
      # (3) u = Auu^{-1}(f_u-Aup*p)

      petsc_options_iname = '-pc_fieldsplit_schur_fact_type  -pc_fieldsplit_schur_precondition -ksp_gmres_restart -ksp_rtol -ksp_type'
      petsc_options_value = 'full                            selfp                             300                1e-4      fgmres'
    []
    [u]
      vars = 'vel_x vel_y'
      petsc_options_iname = '-pc_type -pc_hypre_type -ksp_type -ksp_rtol -ksp_gmres_restart -ksp_pc_side'
      petsc_options_value = 'hypre    boomeramg      gmres    5e-1      300                 right'
    []
    [p]
      vars = 'pressure'
      petsc_options_iname = '-ksp_type -ksp_gmres_restart -ksp_rtol -pc_type -ksp_pc_side'
      petsc_options_value = 'gmres    300                5e-1      jacobi    right'
    []
  []
  [SMP]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -pc_factor_shift_type'
    petsc_options_value = 'lu       NONZERO'
  []
[]

[Outputs]
  print_linear_residuals = true
  print_nonlinear_residuals = true
  [out]
    type = Exodus
    hide = 'Re lin cum_lin'
  []
  [perf]
    type = PerfGraphOutput
  []
  [csv]
    type = CSV
    execute_on = 'FINAL'
  []
[]

[Postprocessors]
  [Re]
    type = ParsedPostprocessor
    function = '${rho} * ${D} * ${U} / ${mu}'
    pp_names = ''
  []
  [lin]
    type = NumLinearIterations
  []
  [cum_lin]
    type = CumulativeValuePostprocessor
    postprocessor = lin
  []
  [limit_dt]
    type = Receiver
    default = 1e5
  []
[]

[VectorPostprocessors]
  [central_velocity]
    type = LineValueSampler
    variable = 'vel_x'
    start_point = '${fparse 1*D} 0 0'
    end_point = '${fparse 25*D} 0 0'
    num_points = 25
    sort_by = 'x'
    execute_on = 'FINAL'
  []
[]
