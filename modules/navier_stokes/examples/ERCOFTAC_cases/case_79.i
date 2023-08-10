# a Pronghorn mesh for Turbulent Natural Convection in an Enclosed Tall Cavity
# Experiments by Betts and Bokhari
DX = 0.076
DY = 2.18
DZ = 0.52

T_hot = '${fparse 273.15 + 19.6}'
T_cold = '${fparse 273.15 + 0.0}'
T_ref = '${fparse (T_hot + T_cold)/2.0}'

rho = 1.293 # kg/m3
cp = 1005 # J/(kg.K)
mu = 1.81e-5 # Pa.s
nu = '${fparse mu/rho}'
k = 0.0257 # W/(m.K)
alpha = '${fparse k/(rho*cp)}'
g = 9.81
Ra = 8.6e5
alpha_b = '${fparse Ra*nu*alpha/(g*(T_hot-T_cold)*DX*DX*DX)}'

# mixing_length = 0.0012

velocity_interp_method = 'rc'
advected_interp_method = 'upwind'

delta = '${fparse 2/1000}'

[Mesh]
  uniform_refine = 0
  [left_mesh]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 25
    ny = 300
    xmax = 0.018
    xmin = 0.0
    ymax = '${DY}'
    bias_x = 1.2
  []
  [center_mesh]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 20
    ny = 300
    xmax = 0.058
    xmin = 0.018
    ymax = '${DY}'
  []
  [right_mesh]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 25
    ny = 300
    xmax = 0.076
    xmin = 0.058
    ymax = '${DY}'
    bias_x = 0.8
  []
  [rename_center]
    type = RenameBoundaryGenerator
    input = center_mesh
    old_boundary = 'bottom top'
    new_boundary = 'center_bottom center_top'
  []
  [rename_right]
    type = RenameBoundaryGenerator
    input = right_mesh
    old_boundary = 'bottom top'
    new_boundary = 'right_bottom right_top'
  []
  [all]
    type = StitchedMeshGenerator
    inputs = 'left_mesh rename_center rename_right'
    clear_stitched_boundary_ids = true
    stitch_boundaries_pairs = 'right left;
                                right left'
  []
  # [Cavity]
  #   type = AdvancedExtruderGenerator
  #   direction = '0 0 1'
  #   input = all
  #   heights = '${DZ}'
  #   num_layers = 10
  # []
  # [rename_sidesets]
  #   type = RenameBoundaryGenerator
  #   input = 'Cavity'
  #   old_boundary = 'top center_top right_top bottom center_bottom right    left      9         10'
  #   new_boundary = 'top top        top       bottom bottom        hot_wall cold_wall side_wall side_wall'
  # []
  [rename_sidesets]
    type = RenameBoundaryGenerator
    input = 'all'
    old_boundary = 'top center_top right_top bottom center_bottom right    left'
    new_boundary = 'top top        top       bottom bottom        hot_wall cold_wall'
  []
[]

[GlobalParams]
  rhie_chow_user_object = 'rc'
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = vel_x
    v = vel_y
    # w = vel_z
    pressure = pressure
  []
[]

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
    initial_condition = 1e-6
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = 1e-6
  []
  # [vel_z]
  #   type = INSFVVelocityVariable
  # []
  [pressure]
    type = INSFVPressureVariable
    initial_condition = 1e-6
  []
  [T_fluid]
    type = INSFVEnergyVariable
    initial_condition = ${T_ref}
    scaling = 1e-3
  []
  [lambda]
    family = SCALAR
    order = FIRST
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
  [mean_zero_pressure]
    type = FVIntegralValueConstraint
    variable = pressure
    lambda = lambda
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
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
    rho = ${rho}
    momentum_component = 'x'
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_x
    mu = ${mu}
    momentum_component = 'x'
  []
  [u_viscosity_rans]
    type = INSFVMixingLengthReynoldsStress
    variable = vel_x
    rho = ${rho}
    mixing_length = 'mixing_len'
    momentum_component = 'x'
    u = vel_x
    v = vel_y
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = vel_x
    momentum_component = 'x'
    pressure = pressure
  []
  [u_buoyancy]
    type = INSFVMomentumBoussinesq
    variable = vel_x
    T_fluid = T_fluid
    gravity = '0 -9.81 0'
    rho = ${rho}
    ref_temperature = ${T_ref}
    momentum_component = 'x'
  []
  [u_gravity]
    type = INSFVMomentumGravity
    variable = vel_x
    gravity = '0 -9.81 0'
    rho = ${rho}
    momentum_component = 'x'
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
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
    rho = ${rho}
    momentum_component = 'y'
  []
  [v_viscosity_rans]
    type = INSFVMixingLengthReynoldsStress
    variable = vel_y
    rho = ${rho}
    mixing_length = 'mixing_len'
    momentum_component = 'y'
    u = vel_x
    v = vel_y
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
  [v_buoyancy]
    type = INSFVMomentumBoussinesq
    variable = vel_y
    T_fluid = T_fluid
    gravity = '0 -9.81 0'
    rho = ${rho}
    ref_temperature = ${T_ref}
    momentum_component = 'y'
  []
  [v_gravity]
    type = INSFVMomentumGravity
    variable = vel_y
    gravity = '0 -9.81 0'
    rho = ${rho}
    momentum_component = 'y'
  []

  # [w_time]
  #   type = INSFVMomentumTimeDerivative
  #   variable = vel_z
  #   rho = ${rho}
  #   momentum_component = 'z'
  # []
  # [w_advection]
  #   type = INSFVMomentumAdvection
  #   variable = vel_z
  #   velocity_interp_method = ${velocity_interp_method}
  #   advected_interp_method = ${advected_interp_method}
  #   rho = ${rho}
  #   momentum_component = 'z'
  # []
  # [w_viscosity]
  #   type = INSFVMomentumDiffusion
  #   variable = vel_z
  #   mu = ${mu}
  #   momentum_component = 'z'
  # []
  # [w_pressure]
  #   type = INSFVMomentumPressure
  #   variable = vel_z
  #   momentum_component = 'z'
  #   pressure = pressure
  # []
  # [w_buoyancy]
  #   type = INSFVMomentumBoussinesq
  #   variable = vel_z
  #   T_fluid = T_fluid
  #   gravity = '0 -9.81 0'
  #   rho = ${rho}
  #   ref_temperature = ${T_ref}
  #   momentum_component = 'z'
  # []
  # [w_gravity]
  #   type = INSFVMomentumGravity
  #   variable = vel_z
  #   gravity = '0 -9.81 0'
  #   rho = ${rho}
  #   momentum_component = 'z'
  # []

  [temp_time]
    type = INSFVEnergyTimeDerivative
    variable = T_fluid
    rho = ${rho}
    cp = ${cp}
  []
  [temp_conduction]
    type = FVDiffusion
    coeff = 'k'
    variable = T_fluid
  []
  [temp_advection]
    type = INSFVEnergyAdvection
    variable = T_fluid
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
  []
  [temp_rans]
    type = INSFVMixingLengthScalarDiffusion
    variable = T_fluid
    mixing_length = 'mixing_len'
    u = vel_x
    v = vel_y
    schmidt_number = 0.9
  []
[]

[FVBCs]
  [no_slip_x]
    type = INSFVNoSlipWallBC
    variable = vel_x
    # boundary = 'top bottom hot_wall cold_wall side_wall'
    boundary = 'top bottom hot_wall cold_wall'
    function = 0
  []
  [no_slip_y]
    type = INSFVNoSlipWallBC
    variable = vel_y
    # boundary = 'top bottom hot_wall cold_wall side_wall'
    boundary = 'top bottom hot_wall cold_wall'
    function = 0
  []

  # [walls-u]
  #   type = INSFVWallFunctionBC
  #   variable = vel_x
  #   boundary = 'hot_wall cold_wall'
  #   u = vel_x
  #   v = vel_y
  #   mu = ${mu}
  #   rho = ${rho}
  #   momentum_component = 'x'
  # []
  # [walls-v]
  #   type = INSFVWallFunctionBC
  #   variable = vel_y
  #   boundary = 'hot_wall cold_wall'
  #   u = vel_x
  #   v = vel_y
  #   mu = ${mu}
  #   rho = ${rho}
  #   momentum_component = 'y'
  # []

  [T_hot]
    type = FVDirichletBC
    variable = T_fluid
    boundary = 'hot_wall'
    value = ${T_hot}
  []
  [T_cold]
    type = FVDirichletBC
    variable = T_fluid
    boundary = 'cold_wall'
    value = ${T_cold}
  []
[]

[AuxVariables]
  [mixing_len]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
[]

[AuxKernels]
  [mixing_len]
    type = WallDistanceMixingLengthAux
    walls = 'top'
    variable = mixing_len
    execute_on = 'initial'
    von_karman_const = 1.0
    delta = 2e-3
    von_karman_const_0 = 1.0
  []
[]

[Materials]
  [const_functor]
    type = ADGenericFunctorMaterial
    prop_names = 'alpha_b cp k'
    prop_values = '${alpha_b} ${cp} ${k}'
  []
  [ins_fv]
    type = INSFVEnthalpyMaterial
    temperature = 'T_fluid'
    rho = ${rho}
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  #petsc_options_iname = '-pc_type -pc_factor_shift_type -ksp_gmres_restart -snes_linesearch_damping'
  #petsc_options_value = 'lu        NONZERO                   200            1.0'
  line_search = 'none'
  nl_rel_tol = 1e-4
  nl_abs_tol = 1e-7
  l_abs_tol = 1e-8
  nl_max_its = 50

  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1e-4
    optimal_iterations = 8
    iteration_window = 2
    growth_factor = 2
    cutback_factor = 0.5
  []
  end_time = 1e+30
  steady_state_detection = true
  steady_state_tolerance = 1e-07
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
      vars = 'vel_x vel_y vel_z T_fluid'
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
  exodus = true
  [csv]
    type = CSV
    execute_on = 'FINAL'
  []
[]

[VectorPostprocessors]
  [y_0d05]
    type = LineValueSampler
    variable = vel_y
    start_point = '${fparse delta} ${fparse DY*0.05} 0.0'
    end_point = '${fparse DX - delta} ${fparse DY*0.05} 0.0'
    sort_by = 'x'
    num_points = 30
    execute_on = FINAL
  []
  [y_0d1]
    type = LineValueSampler
    variable = vel_y
    start_point = '${fparse delta} ${fparse DY*0.1} 0.0'
    end_point = '${fparse DX - delta} ${fparse DY*0.1} 0.0'
    sort_by = 'x'
    num_points = 30
    execute_on = FINAL
  []
  [y_0d3]
    type = LineValueSampler
    variable = vel_y
    start_point = '${fparse delta} ${fparse DY*0.3} 0.0'
    end_point = '${fparse DX - delta} ${fparse DY*0.3} 0.0'
    sort_by = 'x'
    num_points = 30
    execute_on = FINAL
  []
  [y_0d4]
    type = LineValueSampler
    variable = vel_y
    start_point = '${fparse delta} ${fparse DY*0.4} 0.0'
    end_point = '${fparse DX - delta} ${fparse DY*0.4} 0.0'
    sort_by = 'x'
    num_points = 30
    execute_on = FINAL
  []
  [y_0d5]
    type = LineValueSampler
    variable = vel_y
    start_point = '${fparse delta} ${fparse DY*0.5} 0.0'
    end_point = '${fparse DX - delta} ${fparse DY*0.5} 0.0'
    sort_by = 'x'
    num_points = 30
    execute_on = FINAL
  []
  [y_0d6]
    type = LineValueSampler
    variable = vel_y
    start_point = '${fparse delta} ${fparse DY*0.6} 0.0'
    end_point = '${fparse DX - delta} ${fparse DY*0.6} 0.0'
    sort_by = 'x'
    num_points = 30
    execute_on = FINAL
  []
  [y_0d7]
    type = LineValueSampler
    variable = vel_y
    start_point = '${fparse delta} ${fparse DY*0.7} 0.0'
    end_point = '${fparse DX - delta} ${fparse DY*0.7} 0.0'
    sort_by = 'x'
    num_points = 30
    execute_on = FINAL
  []
  [y_0d9]
    type = LineValueSampler
    variable = vel_y
    start_point = '${fparse delta} ${fparse DY*0.9} 0.0'
    end_point = '${fparse DX - delta} ${fparse DY*0.9} 0.0'
    sort_by = 'x'
    num_points = 30
    execute_on = FINAL
  []
  [y_0d95]
    type = LineValueSampler
    variable = vel_y
    start_point = '${fparse delta} ${fparse DY*0.95} 0.0'
    end_point = '${fparse DX - delta} ${fparse DY*0.95} 0.0'
    sort_by = 'x'
    num_points = 30
    execute_on = FINAL
  []
[]

