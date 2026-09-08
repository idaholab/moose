

length = 1.0
R = 0.025
min = 1.0
Pin = 101300
dP = 10130
Tin = 293.15
rho = 998.2
mu = 0.001002
alpha = 0.0
epsilon = 0.0001
forms = 0.0
pump = 0.0
gravity = 0.0
area = ${fparse 3.14159* ${R}^2}

[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = ${length}
  nx = 1
[]

[Variables]
    [m1]
        family = SCALAR
        initial_condition = ${min}
    []
    [dPc]
        family = SCALAR
        initial_condition = ${dP}
    []
    [T1]
        family = SCALAR
        initial_condition = ${Tin}
    []
[]

[FluidProperties]
  [water]
    type = Water97FluidProperties
  []
[]

[ScalarKernels]
    [pipe1_mom]
        type = CoupledPressureIncompressibleMomentumSPScalarKernel
        variable = 'dPc'
        coupled_mass_flow_rate = 'm1'
        temperatures = 'T1'
        reference_pressure = ${Pin}
        fp = 'water'
        areas = '${area}'
        perimeters = '${fparse 2*3.14159* ${R}}'
        lengths = '${length}'
        alphas = '${alpha}'
        forms_losses = '${forms}'
        pump_pressures = '${pump}'
        roughnesses = '${epsilon}'
        g = ${gravity}
        is_implicit = True
    []
    [pipe1_coupledDT]
      type = CoupledODETimeDerivative
      variable = 'dPc'
      v = 'm1'
    []
    [temp]
      type = ParsedODEKernel
      expression = 'T1 - ${Tin}'
      variable = T1
    []
    [m1k]
      type = ParsedODEKernel
      expression = 'm1 - ${min}'
      variable = m1
    []
[]

[Postprocessors]
  [mdot]
    type = ScalarVariable
    variable = m1
    execute_on = 'TIMESTEP_END'
  []
  [dP]
    type = ScalarVariable
    variable = dPc
    execute_on = 'TIMESTEP_END'
  []
  [Re]
    type = ParsedPostprocessor
    expression = 'abs(mdot) / ${area} * 2 * ${R} / ${mu}'
    pp_names = 'mdot'
    execute_on = 'TIMESTEP_END'
  []
  [f]
    type = ParsedPostprocessor
    expression = '0.25 / ((log10(${epsilon} / 2 / 3.7 / ${R} + 5.74 / Re ^ 0.9 ))^2)'
    pp_names = 'Re'
    execute_on = 'TIMESTEP_END'
  []
  [analytical_dP]
    type = ParsedPostprocessor
    expression = '-${pump} + f * ${length} * mdot * abs(mdot) / 4 / ${R} / ${rho} / ${area}^2 + ${forms} * mdot * abs(mdot) / 2 / ${rho} / ${area}^2 + ${rho} * ${gravity} * ${length} * sin(${alpha})'
    pp_names = 'f mdot'
    execute_on = 'TIMESTEP_END'
  []
  [relative_error]
    type = ParsedPostprocessor
    expression = 'abs((analytical_dP - dP)/analytical_dP)'
    pp_names = 'analytical_dP dP'
    execute_on = 'TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  start_time = 0
  end_time = 100.0
  [TimeStepper]
    type = IterationAdaptiveDT
    growth_factor = 1.4
    dt = 5
  []
  solve_type = 'PJFNK'
  nl_abs_tol = 1e-09
  l_tol = 1e-07
[]

[Outputs]
  perf_graph = true
  [out]
    type = CSV
    execute_on = 'FINAL'
    file_base = 'friction_onlyCP'
  []
[]
