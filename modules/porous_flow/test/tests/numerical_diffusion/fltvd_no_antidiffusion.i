# Using Flux-Limited TVD Advection ala Kuzmin and Turek, but without any antidiffusion
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 100
  xmin = 0
  xmax = 1
[]

[Variables]
  [tracer]
  []
[]

[ICs]
  [tracer]
    type = FunctionIC
    variable = tracer
    function = 'if(x<0.1,0,if(x>0.3,0,1))'
  []
[]

[Kernels]
  [mass_dot]
    type = MassLumpedTimeDerivative
    variable = tracer
  []
  [flux]
    type = FluxLimitedTVDAdvection
    variable = tracer
    advective_flux_calculator = fluo
  []
[]

[UserObjects]
  [fluo]
    type = AdvectiveFluxCalculatorConstantVelocity
    flux_limiter_type = none
    u = tracer
    velocity = '0.1 0 0'
  []
[]

[BCs]
  [no_tracer_on_left]
    type = DirichletBC
    variable = tracer
    value = 0
    boundary = left
  []
  [remove_tracer]
# Ideally, an OutflowBC would be used, but that does not exist in the framework
# In 1D VacuumBC is the same as OutflowBC, with the alpha parameter being twice the velocity
    type = VacuumBC
    boundary = right
    alpha = 0.2 # 2 * velocity
    variable = tracer
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
  [tracer]
    type = LineValueSampler
    start_point = '0 0 0'
    end_point = '1 0 0'
    num_points = 101
    sort_by = x
    variable = tracer
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  end_time = 6
  dt = 6E-1
  nl_abs_tol = 1E-8
  nl_max_its = 500
  timestep_tolerance = 1E-3
[]

[Outputs]
  [out]
    type = CSV
    execute_on = final
  []
[]
