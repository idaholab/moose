# Column of water at rest, open at the top and closed at the bottom, made of two vertical flow
# channels joined by a volume junction. For an isothermal stiffened gas at rest, the hydrostatic
# balance has the closed-form solution used as initial condition and as reference. The junction
# box has a height 'jh' (zero by default), so the junction faces can be away from the junction
# center.

H = 7.0
jh = 0
A = 0.5
T0 = 313.15
p_top = 101325
g = 9.81
gamma = 2.35
cv = 1778.3
p_inf = 7.456e8

[GlobalParams]
  gravity_vector = '0 0 -${g}'
  rdg_slope_reconstruction = minmod
  closures = closures
  fp = water
  initial_T = ${T0}
  initial_p = p_exact_fn
  initial_vel = 0
  scaling_factor_1phase = '1 1 1'
  A = ${A}
  D_h = 0.8
  length = ${fparse H / 2 - jh / 2}
  orientation = '0 0 -1'
[]

[FluidProperties]
  [water]
    type = StiffenedGasFluidProperties
    gamma = ${gamma}
    cv = ${cv}
    p_inf = ${p_inf}
    q = -1.8e5
    q_prime = 0
    mu = 6.53e-4
    k = 0.631
  []
[]

[Closures]
  [closures]
    type = Closures1PhaseTHM
  []
[]

[Functions]
  [p_exact_fn]
    type = ParsedFunction
    expression = '(${p_top} + ${p_inf}) * exp(${g} * (${H} - z) / ((${gamma} - 1) * ${cv} * ${T0})) - ${p_inf}'
  []
[]

[Components]
  [outlet]
    type = Outlet1Phase
    input = 'upper:in'
    p = ${p_top}
  []
  [upper]
    type = FlowChannel1Phase
    position = '0 0 ${H}'
    n_elems = 10
  []
  [junction]
    type = VolumeJunction1Phase
    position = '0 0 ${fparse H / 2}'
    connections = 'upper:out lower:in'
    volume = ${fparse 0.01 + A * jh}
    initial_vel_x = 0
    initial_vel_y = 0
    initial_vel_z = 0
    use_hydrostatic_correction = true
  []
  [lower]
    type = FlowChannel1Phase
    position = '0 0 ${fparse H / 2 - jh / 2}'
    n_elems = 4
  []
  [wall]
    type = SolidWall1Phase
    input = 'lower:out'
  []
[]

[AuxVariables]
  [p_err]
    family = MONOMIAL
    order = CONSTANT
    block = 'upper lower'
  []
[]

[AuxKernels]
  [p_err_ak]
    type = ParsedAux
    variable = p_err
    coupled_variables = 'p'
    functor_names = 'p_exact_fn'
    functor_symbols = 'p_exact'
    expression = 'p - p_exact'
    block = 'upper lower'
  []
[]

[Postprocessors]
  [max_abs_p_err]
    type = ElementExtremeValue
    variable = p_err
    value_type = max_abs
    block = 'upper lower'
  []
  [max_abs_rhouA]
    type = ElementExtremeValue
    variable = rhouA
    value_type = max_abs
    block = 'upper lower'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = bdf2
  end_time = 1
  dt = 0.1
  solve_type = NEWTON
  line_search = basic
  nl_rel_tol = 1e-5
  nl_abs_tol = 1e-2
  nl_max_its = 30
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  file_base = hydrostatic_column_out
  [csv]
    type = CSV
    execute_on = FINAL
  []
[]
