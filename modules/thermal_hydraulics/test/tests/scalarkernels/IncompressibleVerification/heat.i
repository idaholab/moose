

length = 1.0
R = 0.025
min = 1.0
Pin = 101300
dP = -10130
Tin = 293.15
Tout = 323.15
mu = 0.001002
cp = 4185.0
k = 0.598
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
    [T0]
        family = SCALAR
        initial_condition = ${Tin}
    []
    [T1]
        family = SCALAR
        initial_condition = ${Tin}
    []
    [Tw]
        family = SCALAR
        initial_condition = ${Tout}
    []
    [T2]
        family = SCALAR
        initial_condition = ${Tout}
    []
[]

[FluidProperties]
  [water]
    type = Water97FluidProperties
  []
[]

[ScalarKernels]
    [pipe1_mom]
        type = IncompressibleMomentumSPScalarKernel
        variable = 'm1'
        reference_pressure_drop = 'dPc'
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
    [temp1]
      type = IncompressibleEnergySPScalarKernel
      mass_flow_rate = 'm1'
      inlet_temperature = 'T0'
      outlet_temperature = 'T2'
      wall_temperature = 'Tw'
      area = ${area}
      fp = water
      length = ${length}
      perimeter = ${fparse 2*3.14159* ${R}}
      reference_pressure = ${Pin}
      variable = T1
      is_implicit = True
    []
    [temp0]
      type = ParsedODEKernel
      expression = 'T0 - ${Tin}'
      variable = T0
    []
    [temp2]
      type = ParsedODEKernel
      expression = 'T2 - ${Tout}'
      variable = T2
    []
    [walltemp]
      type = ParsedODEKernel
      expression = 'Tw - ${Tout}'
      variable = Tw
    []
    [dPk]
      type = ParsedODEKernel
      expression = 'dPc - ${dP}'
      variable = dPc
    []
[]

[Postprocessors]
  [T]
    type = ScalarVariable
    variable = T1
    execute_on = 'TIMESTEP_END'
  []
  [Re]
    type = ParsedPostprocessor
    expression = 'abs(${min}) / ${area} * 2 * ${R} / ${mu}'
    execute_on = 'TIMESTEP_END'
  []
  [Pr]
    type = ParsedPostprocessor
    expression = '${cp} * ${mu} / ${k}'
    execute_on = 'TIMESTEP_END'
  []
  [h]
    type = ParsedPostprocessor
    expression = '0.023 * Re^0.8 * Pr^0.4'
    pp_names = 'Re Pr'
    execute_on = 'TIMESTEP_END'
  []
  [in]
    type = ParsedPostprocessor
    expression = '1 / 2 * (1 - abs(${min})/${min}) * ${Tout}
                  + 1 / 2 * (1 + abs(${min})/${min}) * ${Tin}'
    execute_on = 'TIMESTEP_END'
  []
  [q]
    type = ParsedPostprocessor
    expression = 'h * ${fparse 2*3.14159* ${R}} / 2 * ( 2 * ${Tout} - T - in)'
    pp_names = 'T h in'
    execute_on = 'TIMESTEP_END'
  []
  [analytical_T]
    type = ParsedPostprocessor
    expression = '1 / abs(${min}) / ${cp} * (q * ${length} - ${min} / 2 * (1 - abs(${min})/${min}) * ${cp} * ${Tout}
                  + ${min} / 2 * (1 + abs(${min})/${min}) * ${cp} * ${Tin}) '
    pp_names = 'T q'
  []
  [relative_error]
    type = ParsedPostprocessor
    expression = 'abs((analytical_T - T)/analytical_T)'
    pp_names = 'analytical_T T'
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
  nl_abs_tol = 1e-08
  l_tol = 1e-07
[]

[Outputs]
  perf_graph = true
  [out]
    type = CSV
    execute_on = 'FINAL'
    file_base = 'heat'
  []
[]
