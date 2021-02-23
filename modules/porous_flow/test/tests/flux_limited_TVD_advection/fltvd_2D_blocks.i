# Using Flux-Limited TVD Advection ala Kuzmin and Turek
# 2D version with blocks
# Top block: tracer is defined here, with velocity = (0.1, 0, 0)
# Central block: tracer is not defined here
# Bottom block: tracer is defined here, with velocity = (-0.1, 0, 0)

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    xmin = 0
    xmax = 1
    ny = 5
    ymin = 0
    ymax = 1
  []
  [top]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0 0.6 0'
    top_right = '1 1 0'
    block_id = 1
  []
  [center]
    input = bottom
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0 0.4 0'
    top_right = '1 0.6 0'
    block_id = 2
  []
  [bottom]
    input = top
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0 0 0'
    top_right = '1 0.6 0'
    block_id = 3
  []
  [split_bdys]
    type = BreakBoundaryOnSubdomainGenerator
    input = center
    boundaries = 'left right'
  []
[]

[GlobalParams]
  block = '1 2 3'
[]

[Variables]
  [tracer]
    block = '1 3'
  []
  [dummy]
  []
[]

[ICs]
  [tracer_top]
    type = FunctionIC
    variable = tracer
    function = 'if(x<0.1 | x>0.3, 0, 1)'
    block = '1'
  []
  [tracer_bot]
    type = FunctionIC
    variable = tracer
    function = 'if(x<0.7 | x > 0.9, 0, 1)'
    block = '3'
  []
[]

[Kernels]
  [mass_dot]
    type = MassLumpedTimeDerivative
    variable = tracer
    block = '1 3'
  []
  [flux_top]
    type = FluxLimitedTVDAdvection
    variable = tracer
    advective_flux_calculator = fluo_top
    block = '1'
  []
  [flux_bot]
    type = FluxLimitedTVDAdvection
    variable = tracer
    advective_flux_calculator = fluo_bot
    block = '3'
  []
  [.dummy]
    type = TimeDerivative
    variable = dummy
  []
[]

[UserObjects]
  [fluo_top]
    type = AdvectiveFluxCalculatorConstantVelocity
    flux_limiter_type = superbee
    u = tracer
    velocity = '0.1 0 0'
    block = '1'
  []
  [fluo_bot]
    type = AdvectiveFluxCalculatorConstantVelocity
    flux_limiter_type = superbee
    u = tracer
    velocity = '-0.1 0 0'
    block = '3'
  []
[]

[BCs]
  [no_tracer_on_left_top]
    type = DirichletBC
    variable = tracer
    value = 0
    boundary = 'left_to_1'
  []
  [remove_tracer_top]
# Ideally, an OutflowBC would be used, but that does not exist in the framework
# In 1D VacuumBC is the same as OutflowBC, with the alpha parameter being twice the velocity
    type = VacuumBC
    boundary = 'right_to_1'
    alpha = 0.2 # 2 * velocity
    variable = tracer
  []
  [no_tracer_on_left_bot]
# Ideally, an OutflowBC would be used, but that does not exist in the framework
# In 1D VacuumBC is the same as OutflowBC, with the alpha parameter being twice the velocity
    type = VacuumBC
    boundary = 'left_to_3'
    alpha = 0.2 # 2 * velocity
    variable = tracer
  []
  [remove_tracer_bot]
    type = DirichletBC
    variable = tracer
    value = 0
    boundary = 'right_to_3'
  []
[]

[Preconditioning]
  active = basic
  [basic]
    type = SMP
    full = true
    petsc_options = '-ksp_diagonal_scale -ksp_diagonal_scale_fix'
    petsc_options_iname = '-pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap'
    petsc_options_value = ' asm      lu           NONZERO                   2'
  []
  [preferred_but_might_not_be_installed]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
    petsc_options_value = ' lu       mumps'
  []
[]

[VectorPostprocessors]
  [tracer_bot]
    type = LineValueSampler
    start_point = '0 0 0'
    end_point = '1 0 0'
    num_points = 11
    sort_by = x
    variable = tracer
  []
  [tracer_top]
    type = LineValueSampler
    start_point = '0 1 0'
    end_point = '1 1 0'
    num_points = 11
    sort_by = x
    variable = tracer
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  end_time = 6
  dt = 6E-2
  timestep_tolerance = 1E-3
[]

[Outputs]
  print_linear_residuals = false
  [out]
    type = CSV
    execute_on = final
  []
[]
