# Test 5 from the following reference:
#
#   F. Daude, P. Galon. A Finite-Volume approach for compressible single- and
#   two-phase flows in flexible pipelines with fluid-structure interaction.
#   Journal of Computational Physics 362 (2018) 375-408.
#
# Also, Test 5 from the following reference:
#
#   F. Daude, R.A. Berry, P. Galon. A Finite-Volume method for compressible
#   non-equilibrium two-phase flows in networks of elastic pipelines using the
#   Baer-Nunziato model.
#   Computational Methods in Applied Mechanical Engineering 354 (2019) 820-849.

[GlobalParams]
  gravity_vector = '0 0 0'

  rdg_slope_reconstruction = none

  fp = fp
  closures = simple_closures
  f = 0

  initial_T = T_ic_fn
  initial_p = p_ic_fn
  initial_vel = 0
[]

[Functions]
  [p_ic_fn]
    type = PiecewiseConstant
    axis = x
    x = '0 ${x_disc}'
    y = '${pL} ${pR}'
  []
  [T_ic_fn]
    type = PiecewiseConstant
    axis = x
    x = '0 ${x_disc}'
    y = '${TL} ${TR}'
  []
[]

[FluidProperties]
  [fp]
    type = StiffenedGasFluidProperties
    gamma = ${gamma}
    p_inf = ${p_inf}
    q = ${q}
    cv = ${cv}
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Postprocessors]
  [dt_cfl]
    type = ADCFLTimeStepSize
    CFL = ${CFL}
    vel_names = 'vel'
    c_names = 'c'
  []
[]

[Executioner]
  type = Transient

  end_time = ${t_end}
  [TimeStepper]
    type = PostprocessorDT
    postprocessor = dt_cfl
  []
  [TimeIntegrator]
    type = ExplicitSSPRungeKutta
    order = 1
  []
  solve_type = LINEAR

  l_tol = 1e-4

  nl_rel_tol = 1e-20
  nl_abs_tol = 1e-8
  nl_max_its = 60
[]

[Outputs]
  [csv]
    type = CSV
    execute_postprocessors_on = 'NONE'
    execute_vector_postprocessors_on = 'FINAL'
    create_final_symlink = true
  []
[]
