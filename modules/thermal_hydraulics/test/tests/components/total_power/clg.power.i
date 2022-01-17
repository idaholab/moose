[Functions]
  [decayheatcurve]
    type = PiecewiseLinear
    x = '0.0 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1.0 1.5 2.0 3.0 4.0 5.0 6.0 8.0 10.0'
    y = '1.0 .8382 .572 .3806 .2792 .2246 .1904 .1672 .1503 .1376 .1275 .1032 .09884
             .09209 .0869 .08271 .07922 .07375 .06967'
  []

  [dts]
    type = PiecewiseLinear
    # this matches the decay heat curve function
    x = '0.0 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1.0 1.5 2.0 3.0 4.0 5.0 6.0 8.0 10.0'
    y = '0.1 0.1 0.1 0.1 0.1 0.1 0.1 0.1 0.1 0.1 0.5 0.5 1.0 1.0 1.0 1.0 2.0 2.0  2.0'
  []
[]

[HeatStructureMaterials]
  [mat]
    type = SolidMaterialProperties
    k = 1
    cp = 1
    rho = 1
  []
[]

[Components]
  [total_power]
    type = TotalPower
    power = 1.
  []

  [ch1:solid]
    type = HeatStructureCylindrical
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 1
    initial_T = 300
    names = '0'
    widths = '1'
    n_part_elems = '1'
    materials = 'mat'
  []
[]

[ControlLogic]
  [reactor_power_control]
    type = TimeFunctionComponentControl
    component = total_power
    parameter = power
    function = decayheatcurve
  []
[]

[Postprocessors]
  [reactor_power]
    type = RealComponentParameterValuePostprocessor
    component = total_power
    parameter = power
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
  scheme = 'bdf2'

  [TimeStepper]
    type = FunctionDT
    function = dts
  []
  abort_on_solve_fail = true

  solve_type = 'PJFNK'
  line_search = 'basic'
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-6
  nl_max_its = 10

  l_tol = 1e-3
  l_max_its = 300

  start_time = 0.0
  end_time = 10
[]

[Outputs]
  csv = true
  show = 'reactor_power'
[]
