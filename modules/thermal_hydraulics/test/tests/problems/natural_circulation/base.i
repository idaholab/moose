# Natural circulation loop
#
# The setup consists of 4 connected 1-m pipes, forming a square:
#
#                  top_pipe
#              *--------------* (1,1)
#              |              |
#              | <-        <- |                | g
#  heated_pipe | <-        <- | cooled_pipe    V
#              | <-        <- |
#              |              |
#        (0,0) *--------------*
#                 bottom_pipe
#
# Heating and cooling occurs in the range z = (0.2 m, 0.8 m) with uniform heat fluxes.

[GlobalParams]
  gravity_vector = '0 0 -9.81'
  length = ${length}
  n_elems = ${n_elems}

  A = ${area}

  initial_T = ${T_ambient}
  initial_p = ${p_initial}
  initial_vel = 0

  fp = fp
  closures = closures
  f = 0
  Hw = ${htc}

  rdg_slope_reconstruction = full
  scaling_factor_1phase = '1 1 1e-5'
[]

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
    emit_on_nan = none
  []
[]

[Closures]
  [closures]
    type = Closures1PhaseSimple
  []
[]

[Functions]
  [heating_flux_fn]
    type = PiecewiseConstant
    axis = z
    x = '0 0.2 0.8'
    y = '0 ${fparse power / (S_heated)} 0'
  []
  [cooling_flux_fn]
    type = PiecewiseConstant
    axis = z
    x = '0 0.2 0.8'
    y = '0 ${fparse -power / (S_cooled)} 0'
  []
[]

[Components]
  [heated_pipe]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '0 0 1'
  []
  [top_pipe]
    type = FlowChannel1Phase
    position = '0 0 1'
    orientation = '1 0 0'
  []
  [cooled_pipe]
    type = FlowChannel1Phase
    position = '1 0 1'
    orientation = '0 0 -1'
  []
  [bottom_pipe]
    type = FlowChannel1Phase
    position = '1 0 0'
    orientation = '-1 0 0'
  []

  [heating]
    type = HeatTransferFromHeatFlux1Phase
    flow_channel = 'heated_pipe'
    q_wall = heating_flux_fn
  []
  [cooling]
    type = HeatTransferFromHeatFlux1Phase
    flow_channel = 'cooled_pipe'
    q_wall = cooling_flux_fn
  []
[]

[Preconditioning]
  [pc]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient

  scheme = bdf2
  start_time = 0
  end_time = 50
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 0.01
    optimal_iterations = 6
    iteration_window = 0
    growth_factor = 1.2
    cutback_factor = 0.8
  []

  steady_state_detection = true

  petsc_options_iname = '-pc_type'
  petsc_options_value = ' lu     '

  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10
  nl_max_its = 15
  l_tol = 1e-4
  l_max_its = 10
[]

[VectorPostprocessors]
  [heated_pipe_vpp]
    type = ElementValueSampler
    block = 'heated_pipe'
    variable = ${output_variables}
    sort_by = z
    execute_on = 'FINAL'
  []
  [top_pipe_vpp]
    type = ElementValueSampler
    block = 'top_pipe'
    variable = ${output_variables}
    sort_by = x
    execute_on = 'FINAL'
  []
  [cooled_pipe_vpp]
    type = ElementValueSampler
    block = 'cooled_pipe'
    variable = ${output_variables}
    sort_by = z
    execute_on = 'FINAL'
  []
  [bottom_pipe_vpp]
    type = ElementValueSampler
    block = 'bottom_pipe'
    variable = ${output_variables}
    sort_by = x
    execute_on = 'FINAL'
  []
[]

[Outputs]
  xml = true
  velocity_as_vector = false
  execute_on = 'FINAL'
[]
