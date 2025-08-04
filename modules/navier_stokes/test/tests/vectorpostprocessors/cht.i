### benchmark sources:
### main source: https://doi.org/10.1115/1.4015778
### dimensions, material props: https://www.mdpi.com/2305-7084/3/2/59
!include header.i
!include mesh.i

[Problem]
  linear_sys_names = 'u_system v_system pressure_system energy_system solid_energy_system'
  previous_nl_solution_required = true
  #restart_file_base = 'cht_out_cp/LATEST'
[]

[UserObjects]
  [rc]
    type = RhieChowMassFlux
    u = vel_x
    v = vel_y
    pressure = pressure
    rho = ${rho_f}
    p_diffusion_kernel = p_diffusion
    block = 'fluid'
    pressure_projection_method = consistent
  []
[]

[Variables]
  [vel_x]
    type = MooseLinearVariableFVReal
#    initial_condition = ${v_in}
    solver_sys = u_system
    block = fluid
  []
  [vel_y]
    type = MooseLinearVariableFVReal
    solver_sys = v_system
#    initial_condition = 0.0
    block = fluid
  []
  [pressure]
    type = MooseLinearVariableFVReal
    solver_sys = pressure_system
#    initial_condition = ${P_out}
    block = fluid
  []
  [T_fluid]
    type = MooseLinearVariableFVReal
    solver_sys = energy_system
    initial_condition = ${T_f}
    block = 'fluid'
  []
  [T_solid]
    type = MooseLinearVariableFVReal
    solver_sys = solid_energy_system
    initial_condition = ${T_s}
    block = 'solid'
  []
[]

[LinearFVKernels]
  [source]
    type = LinearFVSource
    variable = T_solid
    source_density = ${q_s}
    block = 'solid'
  []
  [u_time]
    type = LinearFVTimeDerivative
    variable = vel_x
    factor = ${rho_f}
  []
  [v_time]
    type = LinearFVTimeDerivative
    variable = vel_y
    factor = ${rho_f}
  []
  [tf_time]
    type = LinearFVTimeDerivative
    variable = T_fluid
    factor = ${fparse cp_f*rho_f}
  []
  [ts_time]
    type = LinearFVTimeDerivative
    variable = T_solid
    factor = ${fparse cp_s*rho_s}
  []

  [u_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_x
    advected_interp_method = ${advected_interp_method}
    mu = ${mu}
    u = vel_x
    v = vel_y
    momentum_component = 'x'
    rhie_chow_user_object = 'rc'
    use_nonorthogonal_correction = true
  []
  [v_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_y
    advected_interp_method = ${advected_interp_method}
    mu = ${mu}
    u = vel_x
    v = vel_y
    momentum_component = 'y'
    rhie_chow_user_object = 'rc'
    use_nonorthogonal_correction = true
  []
  [u_pressure]
    type = LinearFVMomentumPressure
    variable = vel_x
    pressure = pressure
    momentum_component = 'x'
  []
  [v_pressure]
    type = LinearFVMomentumPressure
    variable = vel_y
    pressure = pressure
    momentum_component = 'y'
  []

  [p_diffusion]
    type = LinearFVAnisotropicDiffusion
    variable = pressure
    diffusion_tensor = Ainv
    use_nonorthogonal_correction = true
  []
  [HbyA_divergence]
    type = LinearFVDivergence
    variable = pressure
    face_flux = HbyA
    force_boundary_execution = true
  []
  [h_advection]
    type = LinearFVEnergyAdvection
    variable = T_fluid
    advected_quantity = temperature
    cp = ${cp_f}
    advected_interp_method = ${advected_interp_method}
    rhie_chow_user_object = 'rc'
  []
  [fluid-conduction]
    type = LinearFVDiffusion
    variable = T_fluid
    diffusion_coeff = ${k_f}
    use_nonorthogonal_correction = true
  []

  [solid-conduction]
    type = LinearFVDiffusion
    variable = T_solid
    diffusion_coeff = ${k_s}
    use_nonorthogonal_correction = true
  []
[]

[Functions]
  [left_boundary_fn]
    type = ParsedFunction
    expression = 'U*(y-ymin)*(ymax-y)/(ymax-ymin)/(ymax-ymin)/1.666852e-01'
    symbol_names = 'U ymax ymin'
    symbol_values = '${v_in} ${y_max} ${y_min}'
  []
[]

[LinearFVBCs]
# velocity BCs
  [left_boundary-u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    #boundary = 'left_boundary'
    boundary = 'left_boundary'
    variable = vel_x
    functor = 'left_boundary_fn'
  []
  [left_boundary-v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left_boundary'
    variable = vel_y
    functor = '0.0'
  []
  [walls-u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'top_boundary bottom_boundary circle'
    variable = vel_x
    functor = 0.0
  []
  [walls-v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'top_boundary bottom_boundary circle'
    variable = vel_y
    functor = 0.0
  []

  [right_boundary_p]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'right_boundary'
    variable = pressure
    functor = ${P_out}
  []
  [pressure-extrapolation]
    type = LinearFVExtrapolatedPressureBC
    boundary = 'circle top_boundary bottom_boundary'
    variable = pressure
    use_two_term_expansion = true
  []
  [right_boundary_u]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = 'right_boundary'
    variable = vel_x
    use_two_term_expansion = false
  []
  [right_boundary_v]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = 'right_boundary'
    variable = vel_y
    use_two_term_expansion = false
 []

# temperature BCs
  [left_boundary_T]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T_fluid
    functor = ${T_f}
    boundary = 'left_boundary'
  []
  [insulated_fluid]
    type = LinearFVAdvectionDiffusionFunctorNeumannBC
    variable = T_fluid
    functor = 0
    boundary = 'top_boundary bottom_boundary'
  []
  [right_boundary_T]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = T_fluid
    use_two_term_expansion = false
    boundary = 'right_boundary'
  []

# Robin(S)-Dirichlet(F) coupling
  [fluid_solid]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T_fluid
    boundary = circle
    functor = T_solid
  []
  [solid_fluid]
    type = LinearFVConjugateHeatTransferBC
    variable = T_solid
    T_solid = T_solid
    T_fluid = T_fluid
    boundary = circle
    h = ${h_s}
    solid_conductivity = ${k_s}
    fluid_conductivity = ${k_f}
  []
[]

[FunctorMaterials]
  [rhocpT]
    property_name = 'rhocpT'
    type = ParsedFunctorMaterial
    functor_names = 'T_fluid'
    expression = '${rho_f}*${cp_f}*T_fluid'
  []
[]

[Postprocessors]
  [inlet_v_avg]
    type = SideAverageValue
    variable = vel_x
    boundary = left_boundary
  []
  [t_step]
    type = TimestepSize
  []
  [timestep_ctr]
    type = NumTimeSteps
  []
  [h_in]
    type = VolumetricFlowRate
    boundary = 'left_boundary'
    vel_x = vel_x
    vel_y = vel_y
    rhie_chow_user_object = rc
    advected_quantity = 'rhocpT'
    subtract_mesh_velocity = false
  []
  [h_out]
    type = VolumetricFlowRate
    boundary = 'right_boundary'
    vel_x = vel_x
    vel_y = vel_y
    rhie_chow_user_object = rc
    advected_quantity = 'rhocpT'
    advected_interp_method = upwind
    subtract_mesh_velocity = false
  []
  [bulk_tf]
    type = FVScalarBulkValue
    scalar = 'T_fluid'
    velocity = 'vel_x'
    block = 'fluid'
  []
  [source_val_pp]
    type = ConstantPostprocessor
    value = ${q_s} 
  []
[]

[VectorPostprocessors]
  [nu]
    type = InterfaceNusseltSampler
    fluid_temperature = 'T_fluid' #'dummy_t' #
    boundary = 'circle'
    sort_by = x
    characteristic_length = ${dia}
    bulk_temperature = 'bulk_tf'
    execution_order_group = 1
  []
[]
[Executioner]
  #[TimeStepper]
  #  type = IterationAdaptiveDT
  #  dt = 1e-3
  #  optimal_iterations = 10
  #[]
  type = PIMPLE
  dt = ${dt}
  end_time = ${t_end}
  momentum_l_abs_tol     = 1e-8
  pressure_l_abs_tol     = 1e-8
  energy_l_abs_tol       = 1e-8
  solid_energy_l_abs_tol = 1e-8
  momentum_l_tol = 0
  pressure_l_tol = 0
  energy_l_tol = 0
  solid_energy_l_tol = 0
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  energy_system = 'energy_system'
  solid_energy_system = 'solid_energy_system'

  momentum_equation_relaxation = 0.8
  pressure_variable_relaxation = 0.3
  energy_equation_relaxation = 0.5

  num_iterations = 500
  pressure_absolute_tolerance     = 1e-6
  momentum_absolute_tolerance     = 1e-6
  energy_absolute_tolerance       = 1e-6
  solid_energy_absolute_tolerance = 1e-6

  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  energy_petsc_options_value = 'hypre boomeramg'
  solid_energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  solid_energy_petsc_options_value = 'hypre boomeramg'
  print_fields = false
  continue_on_max_its = true
  num_cht_fpi = 10
[]

[Outputs]
   [pgraph]
    type = PerfGraphOutput
    level = 0
#    heaviest_sections = 5
  []
  exodus = true
  csv = true
  [out]
    type = Checkpoint
    time_step_interval = 100
    num_files = 2
    wall_time_interval = 3600 # seconds
  []
#  execute_on = linear
[]
