# Using Flux-Limited TVD Advection ala Kuzmin and Turek, with antidiffusion from superbee flux limiting
# 2D version with velocity = (0.1, 0.2, 0)
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  xmin = 0
  xmax = 1
  ny = 10
  ymin = 0
  ymax = 1
[]

[Variables]
  [tracer]
  []
[]

[ICs]
  [tracer]
    type = FunctionIC
    variable = tracer
    function = 'if(x<0.1 | x > 0.3 | y < 0.1 | y > 0.3, 0, 1)'
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
    flux_limiter_type = superbee
    u = tracer
    velocity = '0.1 0.2 0'
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

[Executioner]
  type = Transient
  solve_type = Newton
  end_time = 2
  dt = 0.1
[]

[Outputs]
  print_linear_residuals = false
  [out]
    type = Exodus
    execute_on = 'initial final'
  []
[]
