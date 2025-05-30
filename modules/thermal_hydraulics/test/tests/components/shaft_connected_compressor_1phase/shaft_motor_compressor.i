area = 0.2359
dt = 1.e-3

[GlobalParams]
  initial_p = 1e5
  initial_T = 288
  initial_vel = 60
  initial_vel_x = 60
  initial_vel_y = 0
  initial_vel_z = 0
  A = ${area}
  A_ref = ${area}
  f = 100
  scaling_factor_1phase = '0.04 0.04 0.04e-5'
  closures = simple_closures
  fp = fp
[]

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Components]
  [compressor]
    type = ShaftConnectedCompressor1Phase
    inlet = 'pipe:out'
    outlet = 'pipe:in'
    position = '0 0 0'
    scaling_factor_rhoEV = 1e-5
    volume = ${fparse area*0.45}
    inertia_coeff = '1 1 1 1'
    inertia_const = 1.61397
    speed_cr_I = 1e12
    speed_cr_fr = 0
    tau_fr_coeff = '0 0 0 0'
    tau_fr_const = 0
    omega_rated = 200
    mdot_rated = 21.74
    rho0_rated = 1.1812
    c0_rated = 340
    speeds = '0.0 0.4 0.5 0.6 0.7 0.8 0.9 1.0 1.1 2'
    Rp_functions = 'Rp00 Rp04 Rp05 Rp06 Rp07 Rp08 Rp09 Rp10 Rp11 Rp11'
    eff_functions = 'eff00 eff04 eff05 eff06 eff07 eff08 eff09 eff10 eff11 eff11'
  []
  [pipe]
    type = FlowChannel1Phase
    position = '0.1 0 0'
    orientation = '1 0 0'
    length = 10
    n_elems = 20
  []

  [motor]
    type = ShaftConnectedMotor
    inertia = 1e2
    torque = 100
  []

  [shaft]
    type = Shaft
    connected_components = 'motor compressor'
    initial_speed = 100
  []
[]


[Functions]
  [Rp00]
    type = PiecewiseLinear
    x = '0 0.3736 0.4216'
    y = '1 0.9701 0.9619'
  []
  [eff00]
    type = PiecewiseLinear
    x = '0 0.3736 0.4216'
    y = '0.001 0.8941 0.6641'
  []

  [Rp04]
    type = PiecewiseLinear
    x = '0.3736 0.3745 0.3753 0.3762 0.3770 0.3919 0.4067 0.4216 0.4826'
    y = '1.0789 1.0779 1.0771 1.0759 1.0749 1.0570 1.0388 1.0204 0.9450'
  []
  [eff04]
    type = PiecewiseLinear
    x = '0.3736 0.3745 0.3753 0.3762 0.3770 0.3919 0.4067 0.4216 0.4826'
    y = '0.8941 0.8929 0.8925 0.8915 0.8901 0.8601 0.7986 0.6641 0.1115'
  []

  [Rp05]
    type = PiecewiseLinear
    x = '0.3736 0.4026 0.4106 0.4186 0.4266 0.4346 0.4426 0.4506 0.4586 0.4666 0.4746 0.4826 0.5941'
    y = '1.2898 1.2442 1.2316 1.2189 1.2066 1.1930 1.1804 1.1677 1.1542 1.1413 1.1279 1.1150 0.9357'
  []
  [eff05]
    type = PiecewiseLinear
    x = '0.3736 0.4026 0.4106 0.4186 0.4266 0.4346 0.4426 0.4506 0.4586 0.4666 0.4746 0.4826 0.5941'
    y = '0.9281 0.9263 0.9258 0.9244 0.9226 0.9211 0.9195 0.9162 0.9116 0.9062 0.8995 0.8914 0.7793'
  []

  [Rp06]
    type = PiecewiseLinear
    x = '0.4026 0.4613 0.4723 0.4834 0.4945 0.5055 0.5166 0.5277 0.5387 0.5609 0.5719 0.583 0.5941 0.7124'
    y = '1.5533 1.4438 1.4232 1.4011 1.3793 1.3589 1.3354 1.3100 1.2867 1.2376 1.2131 1.1887 1.1636 0.896'
  []
  [eff06]
    type = PiecewiseLinear
    x = '0.4026 0.4613 0.4723 0.4834 0.4945 0.5055 0.5166 0.5277 0.5387 0.5609 0.5719 0.583 0.5941 0.7124'
    y = '0.9148 0.9255 0.9275 0.9277 0.9282 0.9295 0.9290 0.9269 0.9242 0.9146 0.9080 0.900 0.8920 0.8061'
  []

  [Rp07]
    type = PiecewiseLinear
    x = '0.4613 0.5447 0.5587 0.5726 0.5866 0.6006 0.6145 0.6285 0.6425 0.6565 0.6704 0.6844 0.6984 0.7124 0.8358'
    y = '1.8740 1.6857 1.6541 1.6168 1.5811 1.5430 1.5067 1.4684 1.4292 1.3891 1.3479 1.3061 1.2628 1.2208 0.8498'
  []
  [eff07]
    type = PiecewiseLinear
    x = '0.4613 0.5447 0.5587 0.5726 0.5866 0.6006 0.6145 0.6285 0.6425 0.6565 0.6704 0.6844 0.6984 0.7124 0.8358'
    y = '0.9004 0.9232 0.9270 0.9294 0.9298 0.9312 0.9310 0.9290 0.9264 0.9225 0.9191 0.9128 0.9030 0.8904 0.7789'
  []

  [Rp08]
    type = PiecewiseLinear
    x = '0.5447 0.6638 0.6810 0.6982 0.7154 0.7326 0.7498 0.7670 0.7842 0.8014 0.8186 0.8358 0.9702'
    y = '2.3005 1.9270 1.8732 1.8195 1.7600 1.7010 1.6357 1.5697 1.5019 1.4327 1.3638 1.2925 0.7347'
  []
  [eff08]
    type = PiecewiseLinear
    x = '0.5447 0.6638 0.6810 0.6982 0.7154 0.7326 0.7498 0.7670 0.7842 0.8014 0.8186 0.8358 0.9702'
    y = '0.9102 0.9276 0.9301 0.9313 0.9319 0.9318 0.9293 0.9256 0.9231 0.9153 0.9040 0.8933 0.8098'
  []

  [Rp09]
    type = PiecewiseLinear
    x = '0.6638 0.7762 0.7938 0.8115 0.8291 0.8467 0.8644 0.8820 0.8997 0.9173 0.9349 0.9526 0.9702 1.1107 1.25120'
    y = '2.6895 2.2892 2.2263 2.1611 2.0887 2.0061 1.9211 1.8302 1.7409 1.6482 1.5593 1.4612 1.3586 0.5422 -0.2742'
  []
  [eff09]
    type = PiecewiseLinear
    x = '0.6638 0.7762 0.7938 0.8115 0.8291 0.8467 0.8644 0.8820 0.8997 0.9173 0.9349 0.9526 0.9702 1.1107 1.2512'
    y = '0.8961 0.9243 0.9288 0.9323 0.9330 0.9325 0.9319 0.9284 0.9254 0.9215 0.9134 0.9051 0.8864 0.7380 0.5896'
  []

  [Rp10]
    type = PiecewiseLinear
    x = '0.7762 0.9255 0.9284 0.9461 0.9546 0.9816 0.9968 1.0170 1.039 1.0525 1.0812 1.0880 1.1056 1.1107 1.2511'
    y = '3.3162 2.6391 2.6261 2.5425 2.5000 2.3469 2.2521 2.1211 1.974 1.8806 1.6701 1.6169 1.4710 1.4257 0.1817'
  []
  [eff10]
    type = PiecewiseLinear
    x = '0.7762 0.9255 0.9284 0.9461 0.9546 0.9816 0.9968 1.0170 1.0390 1.0525 1.0812 1.0880 1.1056 1.1107 1.2511'
    y = '0.8991 0.9276 0.9281 0.9308 0.9317 0.9329 0.9318 0.9291 0.9252 0.9223 0.9116 0.9072 0.8913 0.8844 0.6937'
  []

  [Rp11]
    type = PiecewiseLinear
    x = '0.9255 1.0749 1.134 1.2511'
    y = '3.9586 2.9889 2.605 1.4928'
  []
  [eff11]
    type = PiecewiseLinear
    x = '0.9255 1.0749 1.1340 1.2511'
    y = '0.9257 0.9308 0.9328 0.8823'
  []

  [S_energy_fcn]
    type = ParsedFunction
    expression = '-(tau_isen+tau_diss)*omega'
    symbol_names = 'tau_isen tau_diss omega'
    symbol_values = 'isentropic_torque dissipation_torque shaft:omega'
  []
  [energy_conservation_fcn]
    type = ParsedFunction
    expression = '(E_change - S_energy * dt) / E_tot'
    symbol_names = 'E_change S_energy dt E_tot'
    symbol_values = 'E_change S_energy ${dt} E_tot'
  []
[]

[Postprocessors]
  [isentropic_torque]
    type = ElementAverageValue
    variable = isentropic_torque
    block = 'compressor'
    execute_on = 'initial timestep_end'
  []
  [dissipation_torque]
    type = ElementAverageValue
    variable = dissipation_torque
    block = 'compressor'
    execute_on = 'initial timestep_end'
  []

  # mass conservation
  [mass_pipes]
    type = ElementIntegralVariablePostprocessor
    variable = rhoA
    block = 'pipe'
    execute_on = 'initial timestep_end'
  []
  [mass_compressor]
    type = ElementAverageValue
    variable = rhoV
    block = 'compressor'
    execute_on = 'initial timestep_end'
  []
  [mass_tot]
    type = SumPostprocessor
    values = 'mass_pipes mass_compressor'
    execute_on = 'initial timestep_end'
  []
  [mass_conservation]
    type = ChangeOverTimePostprocessor
    postprocessor = mass_tot
    change_with_respect_to_initial = true
    compute_relative_change = true
    execute_on = 'initial timestep_end'
  []

  # energy conservation
  [E_pipes]
    type = ElementIntegralVariablePostprocessor
    variable = rhoEA
    block = 'pipe'
    execute_on = 'initial timestep_end'
  []
  [E_compressor]
    type = ElementAverageValue
    variable = rhoEV
    block = 'compressor'
    execute_on = 'initial timestep_end'
  []
  [E_tot]
    type = LinearCombinationPostprocessor
    pp_coefs = '1 1'
    pp_names = 'E_pipes E_compressor'
    execute_on = 'initial timestep_end'
  []
  [S_energy]
    type = FunctionValuePostprocessor
    function = S_energy_fcn
    indirect_dependencies = 'isentropic_torque dissipation_torque'
    execute_on = 'initial timestep_end'
  []
  [E_change]
    type = ChangeOverTimePostprocessor
    postprocessor = E_tot
    execute_on = 'initial timestep_end'
  []

  # This should also execute on initial. This value is
  # lagged by one timestep as a workaround to moose issue #13262.
  [energy_conservation]
    type = FunctionValuePostprocessor
    function = energy_conservation_fcn
    execute_on = 'timestep_end'
    indirect_dependencies = 'E_tot E_change S_energy'
  []
[]

[Preconditioning]
  [SMP_PJFNK]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = 'implicit-euler'

  dt = ${dt}
  num_steps = 6

  solve_type = 'NEWTON'
  line_search = 'basic'
  petsc_options_iname = '-pc_type'
  petsc_options_value = ' lu'
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-6
  nl_max_its = 15

  l_tol = 1e-4
  l_max_its = 10

  [Quadrature]
    type = GAUSS
    order = SECOND
  []
[]

[Outputs]
  velocity_as_vector = false
[]
