# Material properties
rho = 3279.7 # density [kg / m^3]  (@1000K)
mu = 0.5926 # viscosity [Pa s]
#porosity = 1.0

# Numerical scheme parameters
advected_interp_method = 'skewness-corrected'
velocity_interp_method = 'rc'

[GlobalParams]
  rhie_chow_user_object = 'insfv_rhie_chow_interpolator'
  two_term_boundary_expansion = true
  #advected_interp_method = ${advected_interp_method}
  velocity_interp_method = ${velocity_interp_method}
  u = superficial_vel_x
  v = superficial_vel_y
  w = superficial_vel_z
  pressure = pressure
  #porosity = porosity
  #rho = rho
  mu = mu
[]
################################################################################
# GEOMETRY
################################################################################
[Mesh]
  uniform_refine = 0
  [fmg]
    type = FileMeshGenerator
    file = 'pipe_elbow_long_bl.e'
  []
  [scaling]
    type = TransformGenerator
    transform = 'SCALE'
    vector_value = '0.001 0.001 0.001'
    input = 'fmg'
  []
[]
[Problem]
  kernel_coverage_check = false
[]
################################################################################
# EQUATIONS: VARIABLES, KERNELS & BCS
################################################################################
[UserObjects]
  [insfv_rhie_chow_interpolator]
    type = INSFVRhieChowInterpolator
  []
[]

[Variables]
  [pressure]
    type = INSFVPressureVariable
    initial_condition = 0.0
  []
  [superficial_vel_x]
    type = INSFVVelocityVariable
    initial_condition = 1e-8
  []
  [superficial_vel_y]
    type = INSFVVelocityVariable
    initial_condition = 1e-8
  []
  [superficial_vel_z]
    type = INSFVVelocityVariable
    initial_condition = 1e-8
  []
[]

[FVKernels]
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    advected_interp_method = 'skewness-corrected'
    velocity_interp_method = 'rc'
    rho = ${rho}
  []

  # [u_time]
  #   type = INSFVMomentumTimeDerivative
  #   variable = superficial_vel_x
  #   rho = ${rho}
  #   momentum_component = 'x'
  # []
  [u_advection]
    type = INSFVMomentumAdvection
    variable = superficial_vel_x
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'x'
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = superficial_vel_x
    momentum_component = 'x'
  []
  # [u_viscosity_rans]
  #   type = INSFVMixingLengthReynoldsStress
  #   variable = superficial_vel_x
  #   rho = ${rho}
  #   mixing_length = '${fparse 0.07*0.1}'
  #   momentum_component = 'x'
  # []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = superficial_vel_x
    momentum_component = 'x'
    pressure = pressure
  []

  # [v_time]
  #   type = INSFVMomentumTimeDerivative
  #   variable = superficial_vel_y
  #   rho = ${rho}
  #   momentum_component = 'y'
  # []
  [v_advection]
    type = INSFVMomentumAdvection
    variable = superficial_vel_y
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'y'
  []
  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = superficial_vel_y
    momentum_component = 'y'
  []
  # [v_viscosity_rans]
  #   type = INSFVMixingLengthReynoldsStress
  #   variable = superficial_vel_y
  #   rho = ${rho}
  #   mixing_length = '${fparse 0.07*0.1}'
  #   momentum_component = 'y'
  # []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = superficial_vel_y
    momentum_component = 'y'
    pressure = pressure
  []

  # [w_time]
  #   type = INSFVMomentumTimeDerivative
  #   variable = superficial_vel_z
  #   rho = ${rho}
  #   momentum_component = 'z'
  # []
  [w_advection]
    type = INSFVMomentumAdvection
    variable = superficial_vel_z
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'z'
  []
  [w_viscosity]
    type = INSFVMomentumDiffusion
    variable = superficial_vel_z
    momentum_component = 'z'
  []
  # [w_viscosity_rans]
  #   type = INSFVMixingLengthReynoldsStress
  #   variable = superficial_vel_z
  #   rho = ${rho}
  #   mixing_length = '${fparse 0.07*0.1}'
  #   momentum_component = 'z'
  # []
  [w_pressure]
    type = INSFVMomentumPressure
    variable = superficial_vel_z
    momentum_component = 'z'
    pressure = pressure
  []
[]

[FVBCs]
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'inlet'
    variable = superficial_vel_x
    function = '0'
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'inlet'
    variable = superficial_vel_y
    function = '0.001'
  []
  [inlet-w]
    type = INSFVInletVelocityBC
    boundary = 'inlet'
    variable = superficial_vel_z
    function = '0'
  []
  [no-slip-u]
    type = INSFVNoSlipWallBC
    boundary = 'wall'
    variable = superficial_vel_x
    function = 0
  []
  [no-slip-v]
    type = INSFVNoSlipWallBC
    boundary = 'wall'
    variable = superficial_vel_y
    function = 0
  []
  [no-slip-w]
    type = INSFVNoSlipWallBC
    boundary = 'wall'
    variable = superficial_vel_z
    function = 0
  []
  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'outlet'
    variable = pressure
    function = '0'
  []
[]
################################################################################
# MATERIALS
################################################################################
[Functions]
  # [ad_rampdown_mu_func]
  #   type = ParsedFunction
  #   expression = mu*(10.*exp(-3*t)+1)
  #   symbol_names = 'mu'
  #   symbol_values = ${mu}
  # []
  # Duplicate definition to use in postprocessor,
  # we will convert types more in the future and avoid duplicates
  # [rampdown_mu_func]
  #   type = ParsedFunction
  #   expression = mu*(100*exp(-3*t)+1)
  #   symbol_names = 'mu'
  #   symbol_values = ${mu}
  # []
[]
[Materials]
  [mu]
    type = ADGenericFunctorMaterial #defines mu artificially for numerical convergence
    prop_names = 'mu rho' #it converges to the real mu eventually.
    prop_values = '${mu} ${rho}'
  []
[]
################################################################################
# EXECUTION / SOLVE
################################################################################
[Preconditioning]
  active = SMP
  [FSP]
    type = FSP
    # It is the starting point of splitting
    topsplit = 'up' # 'up' should match the following block name
    [up]
      splitting = 'u p' # 'u' and 'p' are the names of subsolvers
      splitting_type = schur
      petsc_options_iname = '-pc_fieldsplit_schur_fact_type  -pc_fieldsplit_schur_precondition -ksp_gmres_restart -ksp_rtol -ksp_type'
      petsc_options_value = 'full                            selfp                             300                1e-4      fgmres'
    []
    [u]
      vars = 'superficial_vel_x superficial_vel_y superficial_vel_z'
      # petsc_options = '-ksp_monitor'
      petsc_options_iname = '-ksp_type -ksp_gmres_restart -ksp_rtol -pc_type -sub_pc_type -ksp_pc_side -sub_pc_factor_shift_type -sub_pc_factor_levels -pc_asm_overlap'
      petsc_options_value = 'gmres     300                4e-1      asm      ilu          right        NONZERO 1 2'
      # petsc_options_iname = '-pc_type -pc_hypre_type -ksp_type -ksp_rtol -ksp_gmres_restart -ksp_pc_side'
      # petsc_options_value = 'hypre    boomeramg      gmres    5e-1      300                 right'
    []
    [p]
      vars = 'pressure lambda'
      # petsc_options = '-ksp_monitor -pc_jacobi_fixdiag'
      # petsc_options = '-ksp_monitor'
      # petsc_options_iname = '-ksp_type -ksp_gmres_restart -ksp_rtol -pc_type -ksp_pc_side -pc_sor_diagonal_shift'
      # petsc_options_value = 'gmres    300                 5e-1      sor      right        1e-15'
      petsc_options_iname = '-ksp_type -ksp_gmres_restart -ksp_rtol -pc_type -sub_pc_type -ksp_pc_side -sub_pc_factor_shift_type -sub_pc_factor_levels -pc_asm_overlap'
      petsc_options_value = 'gmres     300                4e-1      asm      ilu          right        NONZERO 1 2'
      # petsc_options_iname = '-ksp_type -ksp_gmres_restart -ksp_rtol -pc_type -ksp_pc_side'
      # petsc_options_value = 'gmres    300                5e-1      lu    right'
      # petsc_options_iname = '-ksp_type -ksp_gmres_restart -ksp_rtol -pc_type -ksp_pc_side'
      # petsc_options_value = 'gmres    300                5e-1      jacobi    right'
      # petsc_options_iname = '-ksp_type -ksp_gmres_restart -ksp_rtol -pc_type -pc_hypre_type -ksp_pc_side'
      # petsc_options_value = 'gmres     300                5e-1      hypre    euclid         right'
    []
  []
  [SMP]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -pc_factor_shift_type'
    petsc_options_value = 'lu       NONZERO'
  []
[]
[Executioner]
  type = Steady
  # Time-stepping parameters
  # start_time = 0.0
  # end_time = 10.
  # [TimeStepper]
  #   type = IterationAdaptiveDT
  #   optimal_iterations = 10
  #   # dt = 0.01
  #   dt = 0.005
  #   timestep_limiting_postprocessor = 'dt_limit'
  # []
  # Solver parameters
  solve_type = 'NEWTON'
  # petsc_options_iname = '-pc_type -pc_factor_shift_type -ksp_gmres_restart'
  # petsc_options_value = 'lu NONZERO 20'
  line_search = 'none'
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-6
  nl_max_its = 10 # fail early and try again with a shorter time step
  # l_max_its = 80
  # automatic_scaling = true
  # off_diagonals_in_auto_scaling = true
[]
[Debug]
  show_var_residual_norms = true
[]

################################################################################
# SIMULATION OUTPUTS
################################################################################
[Outputs]
  csv = true
  #hide = 'dt_limit'
  checkpoint = true
  [restart]
    type = Exodus
    execute_on = 'timestep_end final'
  []
  # Reduce base output
  print_linear_converged_reason = true
  print_linear_residuals = true
  print_nonlinear_converged_reason = true
[]

[Postprocessors]
  [dt_limit]
    type = Receiver
    default = 1
  []
[]
